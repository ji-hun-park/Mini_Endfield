using Unity.Entities;
using Unity.Transforms;
using Unity.Mathematics;
using UnityEngine;
using System.Collections.Generic;

namespace Endfield.ECS.Systems
{
    /// <summary>
    /// Runtime system that detects CharacterSpawnerRequest entities baked from SubScenes,
    /// uploads their submeshes to Vulkan, and batch-spawns 10,000 moving characters into the DOTS World.
    /// </summary>
    [UpdateInGroup(typeof(InitializationSystemGroup))]
    public partial class CharacterSpawnerSystem : SystemBase
    {
        protected override void OnCreate()
        {
            RequireForUpdate<CharacterSpawnerRequest>();
        }

        protected override void OnUpdate()
        {
            var ecb = new EntityCommandBuffer(Unity.Collections.Allocator.Temp);

            foreach (var (transform, request, entity) in SystemAPI.Query<RefRO<LocalTransform>, RefRO<CharacterSpawnerRequest>>().WithEntityAccess())
            {
                var req = request.ValueRO;
                GameObject prefab = req.Prefab.Value;

                if (prefab != null)
                {
                    CharacterSpawner.SpawnCharactersInternal(
                        EntityManager,
                        prefab,
                        req.SpawnCount,
                        req.SpawnAreaSize,
                        req.SpawnInGrid == 1,
                        req.BaseHeight,
                        req.MoveSpeedMin,
                        req.MoveSpeedMax,
                        req.MovementBoundsSize,
                        transform.ValueRO.Position
                    );
                }
                else if (EntityManager.HasBuffer<SubmeshBakeData>(entity))
                {
                    var submeshBuffer = EntityManager.GetBuffer<SubmeshBakeData>(entity);
                    var submeshList = new List<CharacterSpawner.SubmeshData>(submeshBuffer.Length);

                    for (int i = 0; i < submeshBuffer.Length; i++)
                    {
                        var item = submeshBuffer[i];
                        Mesh m = item.MeshRef.Value;
                        if (m == null) continue;

                        uint meshId = CharacterSpawner.UploadMeshToVulkan(m, item.SubMeshIndex);
                        ulong sortKey = ((ulong)meshId) << 16;

                        submeshList.Add(new CharacterSpawner.SubmeshData
                        {
                            mesh = m,
                            subMeshIndex = item.SubMeshIndex,
                            meshId = meshId,
                            sortKey = sortKey,
                            relTransform = item.RelTransform,
                            boundsCenter = item.BoundsCenter,
                            boundsExtents = item.BoundsExtents
                        });
                    }

                    if (submeshList.Count > 0)
                    {
                        CharacterSpawner.SpawnCharactersFromSubmeshes(
                            EntityManager,
                            submeshList,
                            req.SpawnCount,
                            req.SpawnAreaSize,
                            req.SpawnInGrid == 1,
                            req.BaseHeight,
                            req.MoveSpeedMin,
                            req.MoveSpeedMax,
                            req.MovementBoundsSize,
                            transform.ValueRO.Position
                        );
                    }
                }

                // Destroy the request entity so it only executes once
                ecb.DestroyEntity(entity);
            }

            ecb.Playback(EntityManager);
            ecb.Dispose();
        }
    }
}
