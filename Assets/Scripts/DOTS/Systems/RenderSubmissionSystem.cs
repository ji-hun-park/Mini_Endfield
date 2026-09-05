using Unity.Burst;
using Unity.Collections;
using Unity.Collections.LowLevel.Unsafe;
using Unity.Entities;
using Unity.Jobs;
using Unity.Transforms;
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
        private NativeList<InstanceData> m_PersistentInstances;

        public static int LastTotalCandidateCount = 0;
        public static int LastVisibleInstanceCount = 0;
        public static int LastCulledCount = 0;
        public static float LastCullingRatio = 0f;
        public static float LastPackAndSubmitTimeMs = 0f;

        protected override void OnCreate()
        {
            m_RenderQuery = GetEntityQuery(
                ComponentType.ReadOnly<RenderMeshComponent>(),
                ComponentType.ReadOnly<LocalToWorld>(),
                ComponentType.ReadOnly<VisibilityComponent>()
            );

            m_PersistentInstances = new NativeList<InstanceData>(131072, Allocator.Persistent);

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
            if (m_PersistentInstances.IsCreated)
            {
                m_PersistentInstances.Dispose();
            }
        }

        protected override void OnUpdate()
        {
            int entityCount = m_RenderQuery.CalculateEntityCount();
            LastTotalCandidateCount = entityCount;
            if (entityCount == 0) return;

            Camera cam = Camera.main;
            if (cam == null) return;

            var sw = System.Diagnostics.Stopwatch.StartNew();

            VulkanPluginWrapper.SetResolution(cam.pixelWidth, cam.pixelHeight);

            float4x4 vp = GL.GetGPUProjectionMatrix(cam.projectionMatrix, false) * cam.worldToCameraMatrix;

            // Zero-allocation parallel list capacity guarantee
            if (m_PersistentInstances.Capacity < entityCount)
            {
                m_PersistentInstances.SetCapacity(math.max(entityCount, 65536));
            }
            m_PersistentInstances.Clear();

            var packJob = new PackInstancesJob
            {
                VPMatrix = vp,
                OutputList = m_PersistentInstances.AsParallelWriter()
            };

            Dependency = packJob.ScheduleParallel(m_RenderQuery, Dependency);
            Dependency.Complete();

            int visibleCount = m_PersistentInstances.Length;
            LastVisibleInstanceCount = visibleCount;
            LastCulledCount = entityCount - visibleCount;
            LastCullingRatio = entityCount > 0 ? ((float)LastCulledCount / entityCount) * 100.0f : 0f;

            if (visibleCount > 0)
            {
                unsafe
                {
                    IntPtr ptr = (IntPtr)m_PersistentInstances.GetUnsafePtr();
                    VulkanPluginWrapper.SubmitRenderBatch(ptr, visibleCount);
                }
            }

            sw.Stop();
            LastPackAndSubmitTimeMs = (float)sw.Elapsed.TotalMilliseconds;
        }

        [BurstCompile]
        private partial struct PackInstancesJob : IJobEntity
        {
            [ReadOnly] public float4x4 VPMatrix;
            public NativeList<InstanceData>.ParallelWriter OutputList;

            public void Execute(in LocalToWorld transform, in RenderMeshComponent renderMesh, in VisibilityComponent visibility)
            {
                if (visibility.IsVisible == 1)
                {
                    OutputList.AddNoResize(new InstanceData
                    {




                        MvpMatrix = math.mul(VPMatrix, transform.Value),
                        SortKey = renderMesh.SortKey
                    });
                }
            }
        }
    }

    public struct InstanceData
    {
        public Unity.Mathematics.float4x4 MvpMatrix;
        public ulong SortKey;
    }
}
