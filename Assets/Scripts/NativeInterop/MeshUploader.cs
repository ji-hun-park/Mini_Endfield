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
        private static uint globalMeshIdCounter = 0;

        void Start()
        {
            if (targetPrefab == null) return;

            MeshFilter[] meshFilters = targetPrefab.GetComponentsInChildren<MeshFilter>();
            uint currentMeshId = globalMeshIdCounter;

            foreach (var mf in meshFilters)
            {
                Mesh mesh = mf.sharedMesh;
                if (mesh == null) continue;

                UploadMesh(mesh, currentMeshId);
                
                // Spawn a DOTS Entity for this mesh so the RenderSubmissionSystem picks it up
                CreateEntityForMesh(mf.transform, currentMeshId);

                currentMeshId++; globalMeshIdCounter = currentMeshId;
            }
        }

        private void CreateEntityForMesh(Transform t, uint meshId)
        {
            var entityManager = World.DefaultGameObjectInjectionWorld.EntityManager;
            var entity = entityManager.CreateEntity(
                typeof(RenderMeshComponent),
                typeof(LocalToWorld),
                typeof(LocalTransform),
                typeof(VisibilityComponent)
            );

            // Construct sort key (Material Set 0, Mesh ID)
            ulong sortKey = (ulong)meshId << 16;

            entityManager.SetComponentData(entity, new RenderMeshComponent { SortKey = sortKey });
            entityManager.SetComponentData(entity, new VisibilityComponent { IsVisible = 1 });
            entityManager.SetComponentData(entity, LocalTransform.FromPositionRotationScale(t.position, t.rotation, t.lossyScale.x));
            // LocalToWorld will be updated by DOTS Transform systems
        }

        private void UploadMesh(Mesh targetMesh, uint meshId)
        {
            Vector3[] vertices = targetMesh.vertices;
            Vector2[] uvs = targetMesh.uv;
            int[] indices = targetMesh.triangles;

            if (uvs == null || uvs.Length != vertices.Length)
            {
                uvs = new Vector2[vertices.Length];
            }

            float[] vertexData = new float[vertices.Length * 5];
            for (int i = 0; i < vertices.Length; i++)
            {
                vertexData[i * 5 + 0] = vertices[i].x;
                vertexData[i * 5 + 1] = vertices[i].y;
                vertexData[i * 5 + 2] = vertices[i].z;
                vertexData[i * 5 + 3] = uvs[i].x;
                vertexData[i * 5 + 4] = uvs[i].y;
            }

            uint[] indexData = new uint[indices.Length];
            for (int i = 0; i < indices.Length; i++)
            {
                indexData[i] = (uint)indices[i];
            }

            GCHandle vertexHandle = GCHandle.Alloc(vertexData, GCHandleType.Pinned);
            GCHandle indexHandle = GCHandle.Alloc(indexData, GCHandleType.Pinned);

            try
            {
                VulkanPluginWrapper.LoadMesh(
                    meshId,
                    vertexHandle.AddrOfPinnedObject(),
                    vertices.Length,
                    indexHandle.AddrOfPinnedObject(),
                    indices.Length
                );
                Debug.Log($"[MeshUploader] Uploaded sub-mesh '{targetMesh.name}' with ID {meshId}");
            }
            finally
            {
                vertexHandle.Free();
                indexHandle.Free();
            }
        }
    }
}


