using Unity.Entities;
using Unity.Mathematics;

namespace Endfield.ECS
{
    // A component representing visual data that needs to be sent to the Vulkan backend
    public struct RenderMeshComponent : IComponentData
    {
        public uint MaterialID;
        public uint MeshID;
        // The 64-bit sort key combining Pass, Pipeline, Material, Depth
        public ulong SortKey;
    }

    public struct AABBComponent : IComponentData
    {
        public float3 Center;
        public float3 Extents;
    }

    public struct VisibilityComponent : IComponentData
    {
        // 1 if visible, 0 if culled by Software Occlusion Culling
        public byte IsVisible;
    }
}

