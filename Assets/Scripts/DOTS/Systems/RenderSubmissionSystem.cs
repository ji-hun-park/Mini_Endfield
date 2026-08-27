using Unity.Burst;
using Unity.Collections;
using Unity.Collections.LowLevel.Unsafe;
using Unity.Entities;
using Unity.Jobs;
using Unity.Transforms;
using Unity.Burst.Intrinsics;
using Unity.Mathematics;
using System;
using System.IO;
using Endfield.Rendering;
using UnityEngine.Rendering;
using UnityEngine;

namespace Endfield.ECS.Systems
{
    [UpdateInGroup(typeof(PresentationSystemGroup))]
    [UpdateAfter(typeof(SoftwareCullingSystem))]
    public partial class RenderSubmissionSystem : SystemBase
    {
        private EntityQuery m_RenderQuery;

        protected override void OnCreate()
        {
            m_RenderQuery = GetEntityQuery(
                ComponentType.ReadOnly<RenderMeshComponent>(),
                ComponentType.ReadOnly<LocalToWorld>(),
                ComponentType.ReadOnly<VisibilityComponent>()
            );

            VulkanPluginWrapper.InitializeWithDebug();

            string vertPath = Path.Combine(Application.dataPath, "../vert.spv");
            string fragPath = Path.Combine(Application.dataPath, "../frag.spv");
            if (File.Exists(vertPath) && File.Exists(fragPath))
            {
                byte[] vertCode = File.ReadAllBytes(vertPath);
                byte[] fragCode = File.ReadAllBytes(fragPath);
                VulkanPluginWrapper.SetShaders(vertCode, vertCode.Length, fragCode, fragCode.Length);
            }
        }

        protected override void OnDestroy()
        {
        }

        protected override void OnUpdate()
        {
            int entityCount = m_RenderQuery.CalculateEntityCount();
            if (entityCount == 0) return;

            Camera cam = Camera.main;
            if (cam == null) return;

            VulkanPluginWrapper.SetResolution(cam.pixelWidth, cam.pixelHeight);

            Matrix4x4 vp = GL.GetGPUProjectionMatrix(cam.projectionMatrix, false) * cam.worldToCameraMatrix;

            NativeArray<InstanceData> instanceData = new NativeArray<InstanceData>(entityCount, Allocator.TempJob);
            NativeArray<int> counter = new NativeArray<int>(1, Allocator.TempJob);
            counter[0] = 0;

            var packJob = new PackInstanceDataJob
            {
                Instances = instanceData,
                Counter = counter,
                VPMatrix = vp,
                LocalToWorldType = GetComponentTypeHandle<LocalToWorld>(true),
                RenderMeshType = GetComponentTypeHandle<RenderMeshComponent>(true),
                VisibilityType = GetComponentTypeHandle<VisibilityComponent>(true)
            };

            Dependency = packJob.Schedule(m_RenderQuery, Dependency);
            Dependency.Complete();

            int visibleCount = counter[0];

            if (visibleCount > 0)
            {
                unsafe
                {
                    IntPtr ptr = (IntPtr)NativeArrayUnsafeUtility.GetUnsafePtr(instanceData);
                    VulkanPluginWrapper.SubmitRenderBatch(ptr, visibleCount);
                }
            }

            instanceData.Dispose();
            counter.Dispose();
        }

        [BurstCompile]
        private struct PackInstanceDataJob : IJobChunk
        {
            public NativeArray<InstanceData> Instances;
            public NativeArray<int> Counter;
            public float4x4 VPMatrix;

            [ReadOnly] public ComponentTypeHandle<LocalToWorld> LocalToWorldType;
            [ReadOnly] public ComponentTypeHandle<RenderMeshComponent> RenderMeshType;
            [ReadOnly] public ComponentTypeHandle<VisibilityComponent> VisibilityType;

            public void Execute(in ArchetypeChunk chunk, int unfilteredChunkIndex, bool useEnabledMask, in v128 chunkEnabledMask)
            {
                var matrices = chunk.GetNativeArray(ref LocalToWorldType);
                var renderMeshes = chunk.GetNativeArray(ref RenderMeshType);
                var visibilities = chunk.GetNativeArray(ref VisibilityType);

                int count = chunk.Count;
                int currentIdx = Counter[0];

                for (int i = 0; i < count; i++)
                {
                    if (visibilities[i].IsVisible == 1)
                    {
                        Instances[currentIdx++] = new InstanceData
                        {
                            MvpMatrix = math.mul(VPMatrix, matrices[i].Value),
                            SortKey = renderMeshes[i].SortKey
                        };
                    }
                }

                Counter[0] = currentIdx;
            }
        }
    }

    public struct InstanceData
    {
        public Unity.Mathematics.float4x4 MvpMatrix;
        public ulong SortKey;
    }
}


