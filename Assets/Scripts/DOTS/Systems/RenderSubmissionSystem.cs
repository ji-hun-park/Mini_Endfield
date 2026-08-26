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
            // For production, we'd extract the sorted keys and matrices here
            int entityCount = m_RenderQuery.CalculateEntityCount();

            // Allocate a temporary buffer for native transfer
            NativeArray<InstanceData> instanceData = new NativeArray<InstanceData>(entityCount, Allocator.TempJob);

            // Schedule job to pack data (Sort + Filter)
            var packJob = new PackInstanceDataJob
            {
                Instances = instanceData
            };

            Dependency = packJob.Schedule(m_RenderQuery, Dependency);
            Dependency.Complete();

            // Submit to Vulkan Plugin
            unsafe
            {
                IntPtr ptr = (IntPtr)NativeArrayUnsafeUtility.GetUnsafePtr(instanceData);
                VulkanPluginWrapper.SubmitRenderBatch(ptr, instanceData.Length);
            }

            instanceData.Dispose();

            VulkanPluginWrapper.EndFrame();
        }

        [BurstCompile]
        private struct PackInstanceDataJob : IJobChunk
        {
            // Implementation for packing instance data into the NativeArray
            // Sorting should ideally happen before or during this phase.
            public NativeArray<InstanceData> Instances;

            public void Execute(in ArchetypeChunk chunk, int unfilteredChunkIndex, bool useEnabledMask, in v128 chunkEnabledMask)
            {
                // Iterate through chunks, check VisibilityComponent, and copy matrices & sort keys
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

