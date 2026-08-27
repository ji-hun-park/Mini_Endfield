using Unity.Burst;
using Unity.Collections;
using Unity.Collections.LowLevel.Unsafe;
using Unity.Entities;
using Unity.Jobs;
using Unity.Transforms;
using Unity.Burst.Intrinsics;
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
        private CommandBuffer m_CommandBuffer;

        protected override void OnCreate()
        {
            m_RenderQuery = GetEntityQuery(
                ComponentType.ReadOnly<RenderMeshComponent>(),
                ComponentType.ReadOnly<LocalToWorld>(),
                ComponentType.ReadOnly<VisibilityComponent>()
            );

            VulkanPluginWrapper.InitializeWithDebug();
            
            // Load and send shaders
            string vertPath = Path.Combine(Application.dataPath, "../vert.spv");
            string fragPath = Path.Combine(Application.dataPath, "../frag.spv");
            if (File.Exists(vertPath) && File.Exists(fragPath)) {
                byte[] vertCode = File.ReadAllBytes(vertPath);
                byte[] fragCode = File.ReadAllBytes(fragPath);
                VulkanPluginWrapper.SetShaders(vertCode, vertCode.Length, fragCode, fragCode.Length);
            }

            m_CommandBuffer = new CommandBuffer();
            m_CommandBuffer.name = "Native Vulkan Render Pass";
        }

        protected override void OnDestroy()
        {
            if (m_CommandBuffer != null)
            {
                m_CommandBuffer.Release();
            }
        }

        protected override void OnUpdate()
        {
            // Collect all visible entities
            int entityCount = m_RenderQuery.CalculateEntityCount();
            if (entityCount == 0) return;

            NativeArray<InstanceData> instanceData = new NativeArray<InstanceData>(entityCount, Allocator.TempJob);
            NativeArray<int> counter = new NativeArray<int>(1, Allocator.TempJob);
            counter[0] = 0;

            var packJob = new PackInstanceDataJob
            {
                Instances = instanceData,
                Counter = counter,
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
                    
                    m_CommandBuffer.Clear();
                    m_CommandBuffer.IssuePluginEvent(VulkanPluginWrapper.GetRenderEventFunc(), 1);
                    Graphics.ExecuteCommandBuffer(m_CommandBuffer);
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
                            WorldMatrix = matrices[i].Value,
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
        public Unity.Mathematics.float4x4 WorldMatrix;
        public ulong SortKey;
    }
}
