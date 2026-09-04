using UnityEngine;
using System;
using System.Runtime.InteropServices;
using Endfield.Rendering;
using Endfield.ECS;
using Unity.Entities;
using Unity.Transforms;

namespace Endfield.ECS.Systems
{
    public class MeshUploader : MonoBehaviour
    {
        public GameObject targetPrefab; // Can be a prefab or a model in the scene

        void Start()
        {
            if (targetPrefab == null) return;

            MeshFilter[] meshFilters = targetPrefab.GetComponentsInChildren<MeshFilter>();

            foreach (var mf in meshFilters)
            {
                Mesh mesh = mf.sharedMesh;
                if (mesh == null) continue;

                uint meshId = CharacterSpawner.UploadMeshToVulkan(mesh);
                
                // Spawn a DOTS Entity for this mesh so the RenderSubmissionSystem picks it up
                CreateEntityForMesh(mf.transform, meshId);
            }
        }

        private void CreateEntityForMesh(Transform t, uint meshId)
        {
            var world = World.DefaultGameObjectInjectionWorld;
            if (world == null) return;
            var entityManager = world.EntityManager;

            var entity = entityManager.CreateEntity(
                typeof(RenderMeshComponent),
                typeof(LocalToWorld),
                typeof(LocalTransform),
                typeof(VisibilityComponent),
                typeof(AABBComponent)
            );

            // Construct sort key (Material Set 0, Mesh ID)
            ulong sortKey = (ulong)meshId << 16;

            entityManager.SetComponentData(entity, new RenderMeshComponent { SortKey = sortKey, MeshID = meshId });
            entityManager.SetComponentData(entity, new VisibilityComponent { IsVisible = 1 });
            entityManager.SetComponentData(entity, LocalTransform.FromPositionRotationScale(t.position, t.rotation, t.lossyScale.x));
            entityManager.SetComponentData(entity, new AABBComponent
            {
                Center = Vector3.zero,
                Extents = new Vector3(2f, 2f, 2f)
            });
        }
    }
}
