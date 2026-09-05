using Unity.Burst;
using Unity.Collections;
using Unity.Entities;
using Unity.Jobs;
using Unity.Mathematics;
using Unity.Transforms;

namespace Endfield.ECS.Systems
{
    // Replaces the custom C++ Culling implementation with Unity DOTS Job System
    [UpdateInGroup(typeof(PresentationSystemGroup))]
    public partial class SoftwareCullingSystem : SystemBase
    {
        public static bool EnableCulling = true;
        public static float LastCullingTimeMs = 0f;
        public static float MaxRenderDistance = 1000f;

        protected override void OnUpdate()
        {
            var camera = UnityEngine.Camera.main;
            if (camera == null) return;

            var sw = System.Diagnostics.Stopwatch.StartNew();

            // 1. Get Frustum Planes from Camera
            float3 cameraPos = camera.transform.position;
            var unityPlanes = UnityEngine.GeometryUtility.CalculateFrustumPlanes(camera);

            NativeArray<float4> frustumPlanes = new NativeArray<float4>(6, Allocator.TempJob);
            for (int i = 0; i < 6; i++)
            {
                frustumPlanes[i] = new float4(unityPlanes[i].normal.x, unityPlanes[i].normal.y, unityPlanes[i].normal.z, unityPlanes[i].distance);
            }

            var cullingJob = new CullingJob
            {
                CameraPosition = cameraPos,
                FrustumPlanes = frustumPlanes,
                EnableCulling = EnableCulling ? (byte)1 : (byte)0,
                MaxRenderDistanceSq = MaxRenderDistance * MaxRenderDistance
            };

            Dependency = cullingJob.ScheduleParallel(Dependency);
            sw.Stop();
            LastCullingTimeMs = (float)sw.Elapsed.TotalMilliseconds;
        }

        [BurstCompile]
        private partial struct CullingJob : IJobEntity
        {
            public float3 CameraPosition;
            public byte EnableCulling;
            public float MaxRenderDistanceSq;

            [DeallocateOnJobCompletion]
            [ReadOnly] public NativeArray<float4> FrustumPlanes;

            public void Execute(in LocalToWorld transform, in AABBComponent aabb, ref RenderMeshComponent renderMesh, ref VisibilityComponent visibility)
            {
                // 1. Calculate World AABB
                float3 worldCenter = math.transform(transform.Value, aabb.Center);

                // Approximate world extents using basis vectors
                float3 right = transform.Right * aabb.Extents.x;
                float3 up = transform.Up * aabb.Extents.y;
                float3 forward = transform.Forward * aabb.Extents.z;
                float3 worldExtents = math.abs(right) + math.abs(up) + math.abs(forward);

                // 2. Frustum Culling & View Distance Culling (Phase 1)
                bool isVisible = true;
                if (EnableCulling == 1)
                {
                    float distSq = math.distancesq(worldCenter, CameraPosition);
                    if (distSq > MaxRenderDistanceSq)
                    {
                        isVisible = false;
                    }
                    else
                    {
                        for (int i = 0; i < 6; i++)
                        {
                            float4 plane = FrustumPlanes[i];
                            float3 normal = plane.xyz;
                            float distance = plane.w;

                            float r = math.dot(worldExtents, math.abs(normal));
                            float d = math.dot(normal, worldCenter) + distance;

                            if (d < -r)
                            {
                                isVisible = false;
                                break;
                            }
                        }
                    }
                }

                // 3. Update visibility
                visibility.IsVisible = (byte)(isVisible ? 1 : 0);

                // 4. Calculate depth for Sort Key
                if (isVisible)
                {
                    float dist = math.distance(transform.Position, CameraPosition);

                    // Simple depth quantization (0 to 65535, mapping 0~1000m)
                    // You can flip it if you want front-to-back sorting
                    ulong depthVal = (ulong)math.clamp((dist / 1000f) * 65535f, 0, 65535);

                    // Update the lower 16 bits of SortKey (Depth)
                    renderMesh.SortKey = (renderMesh.SortKey & 0xFFFFFFFFFFFF0000ul) | (depthVal & 0xFFFFul);
                }
            }
        }
    }
}

