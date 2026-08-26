using Unity.Burst;
using Unity.Collections;
using Unity.Collections.LowLevel.Unsafe;
using Unity.Entities;
using Unity.Jobs;
using Unity.Transforms;
using Unity.Burst.Intrinsics;
using System;
using Endfield.Rendering;

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

            // Initialize Vulkan when system is created
            VulkanPluginWrapper.InitializeVulkanBackend();
            VulkanPluginWrapper.SetupRenderGraph();
        }

        protected override void OnDestroy()
        {
            VulkanPluginWrapper.ShutdownVulkanBackend();
        }

        protected override void OnUpdate()
        {
            VulkanPluginWrapper.BeginFrame();

            // Collect all visible entities
            int entityCount = m_RenderQuery.CalculateEntityCount();

            if (entityCount == 0)
            {
                VulkanPluginWrapper.EndFrame();
                return;
            }

            // Allocate a temporary buffer for native transfer and a counter
            NativeArray<InstanceData> instanceData = new NativeArray<InstanceData>(entityCount, Allocator.TempJob);
            NativeArray<int> counter = new NativeArray<int>(1, Allocator.TempJob);
            counter[0] = 0;

            // Schedule job to pack data (Filter visible + copy)
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

            // Submit to Vulkan Plugin only if there's anything visible
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

            VulkanPluginWrapper.EndFrame();
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

    // Structure that matches the C++ expected input
    public struct InstanceData
    {
        public Unity.Mathematics.float4x4 WorldMatrix;
        public ulong SortKey;
    }
}

