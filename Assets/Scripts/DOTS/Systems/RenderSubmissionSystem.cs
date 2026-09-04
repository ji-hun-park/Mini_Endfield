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

        // Persistent Zero-Allocation Buffers
        private NativeArray<InstanceData> m_PersistentInstances;
        private NativeArray<int> m_ChunkVisibleCounts;
        private NativeArray<int> m_ChunkOffsets;
        private NativeArray<int> m_TotalVisibleCounter;

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

            m_TotalVisibleCounter = new NativeArray<int>(1, Allocator.Persistent);

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
            if (m_PersistentInstances.IsCreated) m_PersistentInstances.Dispose();
            if (m_ChunkVisibleCounts.IsCreated) m_ChunkVisibleCounts.Dispose();
            if (m_ChunkOffsets.IsCreated) m_ChunkOffsets.Dispose();
            if (m_TotalVisibleCounter.IsCreated) m_TotalVisibleCounter.Dispose();
        }

        protected override void OnUpdate()
        {
            int entityCount = m_RenderQuery.CalculateEntityCount();
            LastTotalCandidateCount = entityCount;
            if (entityCount == 0) return;

            int chunkCount = m_RenderQuery.CalculateChunkCount();
            if (chunkCount == 0) return;

            Camera cam = Camera.main;
            if (cam == null) return;

            var sw = System.Diagnostics.Stopwatch.StartNew();

            VulkanPluginWrapper.SetResolution(cam.pixelWidth, cam.pixelHeight);

            float4x4 vp = GL.GetGPUProjectionMatrix(cam.projectionMatrix, false) * cam.worldToCameraMatrix;

            // Zero-allocation persistent buffer resizing
            if (!m_PersistentInstances.IsCreated || m_PersistentInstances.Length < entityCount)
            {
                if (m_PersistentInstances.IsCreated) m_PersistentInstances.Dispose();
                int newCap = math.max(entityCount, 65536);
                m_PersistentInstances = new NativeArray<InstanceData>(newCap, Allocator.Persistent);
            }

            if (!m_ChunkVisibleCounts.IsCreated || m_ChunkVisibleCounts.Length < chunkCount)
            {
                if (m_ChunkVisibleCounts.IsCreated) m_ChunkVisibleCounts.Dispose();
                if (m_ChunkOffsets.IsCreated) m_ChunkOffsets.Dispose();
                int newCap = math.max(chunkCount, 1024);
                m_ChunkVisibleCounts = new NativeArray<int>(newCap, Allocator.Persistent);
                m_ChunkOffsets = new NativeArray<int>(newCap, Allocator.Persistent);
            }

            // Step 1: Parallel visible entity count per chunk
            var countJob = new CountChunkVisibleJob
            {
                VisibilityType = GetComponentTypeHandle<VisibilityComponent>(true),
                ChunkCounts = m_ChunkVisibleCounts
            };
            Dependency = countJob.ScheduleParallel(m_RenderQuery, Dependency);

            // Step 2: Sequential prefix sum of offsets (~0.001 ms in Burst)
            var prefixJob = new PrefixSumJob
            {
                ChunkCounts = m_ChunkVisibleCounts,
                ChunkOffsets = m_ChunkOffsets,
                TotalVisibleCount = m_TotalVisibleCounter,
                NumChunks = chunkCount
            };
            Dependency = prefixJob.Schedule(Dependency);

            // Step 3: Fully parallelized instance packing with AVX2 SIMD matrix multiplications
            var packJob = new PackInstancesParallelJob
            {
                LocalToWorldType = GetComponentTypeHandle<LocalToWorld>(true),
                RenderMeshType = GetComponentTypeHandle<RenderMeshComponent>(true),
                VisibilityType = GetComponentTypeHandle<VisibilityComponent>(true),
                ChunkOffsets = m_ChunkOffsets,
                VPMatrix = vp,
                Instances = m_PersistentInstances
            };
            Dependency = packJob.ScheduleParallel(m_RenderQuery, Dependency);
            Dependency.Complete();

            int visibleCount = m_TotalVisibleCounter[0];
            LastVisibleInstanceCount = visibleCount;
            LastCulledCount = entityCount - visibleCount;
            LastCullingRatio = entityCount > 0 ? ((float)LastCulledCount / entityCount) * 100.0f : 0f;

            if (visibleCount > 0)
            {
                unsafe
                {
                    IntPtr ptr = (IntPtr)NativeArrayUnsafeUtility.GetUnsafePtr(m_PersistentInstances);
                    VulkanPluginWrapper.SubmitRenderBatch(ptr, visibleCount);
                }
            }

            sw.Stop();
            LastPackAndSubmitTimeMs = (float)sw.Elapsed.TotalMilliseconds;
        }

        [BurstCompile]
        private struct CountChunkVisibleJob : IJobChunk
        {
            [ReadOnly] public ComponentTypeHandle<VisibilityComponent> VisibilityType;
            [NativeDisableParallelForRestriction] public NativeArray<int> ChunkCounts;

            public void Execute(in ArchetypeChunk chunk, int unfilteredChunkIndex, bool useEnabledMask, in v128 chunkEnabledMask)
            {
                var visibilities = chunk.GetNativeArray(ref VisibilityType);
                int count = chunk.Count;
                int visible = 0;
                for (int i = 0; i < count; i++)
                {
                    if (visibilities[i].IsVisible == 1)
                    {
                        visible++;
                    }
                }
                ChunkCounts[unfilteredChunkIndex] = visible;
            }
        }

        [BurstCompile]
        private struct PrefixSumJob : IJob
        {
            [ReadOnly] public NativeArray<int> ChunkCounts;
            public NativeArray<int> ChunkOffsets;
            public NativeArray<int> TotalVisibleCount;
            public int NumChunks;

            public void Execute()
            {
                int sum = 0;
                for (int i = 0; i < NumChunks; i++)
                {
                    ChunkOffsets[i] = sum;
                    sum += ChunkCounts[i];
                }
                TotalVisibleCount[0] = sum;
            }
        }

        [BurstCompile]
        private struct PackInstancesParallelJob : IJobChunk
        {
            [ReadOnly] public ComponentTypeHandle<LocalToWorld> LocalToWorldType;
            [ReadOnly] public ComponentTypeHandle<RenderMeshComponent> RenderMeshType;
            [ReadOnly] public ComponentTypeHandle<VisibilityComponent> VisibilityType;
            [ReadOnly] public NativeArray<int> ChunkOffsets;
            public float4x4 VPMatrix;

            [NativeDisableParallelForRestriction]
            public NativeArray<InstanceData> Instances;

            public void Execute(in ArchetypeChunk chunk, int unfilteredChunkIndex, bool useEnabledMask, in v128 chunkEnabledMask)
            {
                var matrices = chunk.GetNativeArray(ref LocalToWorldType);
                var renderMeshes = chunk.GetNativeArray(ref RenderMeshType);
                var visibilities = chunk.GetNativeArray(ref VisibilityType);

                int count = chunk.Count;
                int writeIdx = ChunkOffsets[unfilteredChunkIndex];

                for (int i = 0; i < count; i++)
                {
                    if (visibilities[i].IsVisible == 1)
                    {
                        Instances[writeIdx++] = new InstanceData
                        {
                            MvpMatrix = math.mul(VPMatrix, matrices[i].Value),
                            SortKey = renderMeshes[i].SortKey
                        };
                    }
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
