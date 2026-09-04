using UnityEngine;
using Unity.Entities;
using Unity.Transforms;
using Unity.Mathematics;
using Unity.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using Endfield.Rendering;
using Endfield.ECS;

namespace Endfield.ECS.Systems
{
    /// <summary>
    /// Spawner that extracts submeshes from an FBX model, uploads them to the Vulkan backend once,
    /// and batch-spawns 10,000+ character instances into the DOTS ECS World with movement scripts.
    /// Works both in standard GameObjects (Main Scene) and via Baking in DOTS SubScenes!
    /// </summary>
    public class CharacterSpawner : MonoBehaviour
    {
        [Header("FBX / Character Model")]
        [Tooltip("The FBX model or prefab with submeshes (e.g. 42.fbx)")]
        public GameObject characterPrefab;

        [Header("Spawn Configuration")]
        [Tooltip("Number of characters to spawn (default 10,000)")]
        public int spawnCount = 10000;

        [Tooltip("Size of the area in which characters spawn (X and Z)")]
        public Vector2 spawnAreaSize = new Vector2(250f, 250f);

        [Tooltip("If true, spawns characters in a uniform grid; if false, randomly scatters them")]
        public bool spawnInGrid = false;

        [Tooltip("Base Y height for spawning")]
        public float baseHeight = 0f;

        [Header("Movement Configuration")]
        [Tooltip("Minimum movement speed (m/s)")]
        public float moveSpeedMin = 3f;

        [Tooltip("Maximum movement speed (m/s)")]
        public float moveSpeedMax = 8f;

        [Tooltip("Bounding area for character wandering and wall reflections")]
        public Vector2 movementBoundsSize = new Vector2(280f, 280f);

        [Header("Lifecycle")]
        [Tooltip("Automatically spawn when the scene starts")]
        public bool autoSpawnOnStart = true;

        // Static cache for uploaded meshes to guarantee no duplicate Vulkan buffers are created
        private static readonly Dictionary<Mesh, uint> s_UploadedMeshCache = new Dictionary<Mesh, uint>();
        public static uint GlobalMeshIdCounter = 0;

        public struct SubmeshData
        {
            public Mesh mesh;
            public uint meshId;
            public ulong sortKey;
            public LocalTransform relTransform;
            public float3 boundsCenter;
            public float3 boundsExtents;
        }

        private void Reset()
        {
#if UNITY_EDITOR
            if (characterPrefab == null)
            {
                characterPrefab = UnityEditor.AssetDatabase.LoadAssetAtPath<GameObject>("Assets/FBX/42.fbx");
            }
#endif
        }

        private void Start()
        {
            // Note: In a SubScene, MonoBehaviour Start() is NOT called.
            // Start() only runs if CharacterSpawner is placed in the Main Scene (GameObject mode).
            if (autoSpawnOnStart)
            {
                SpawnCharacters();
            }
        }

        [ContextMenu("Spawn Characters")]
        public void SpawnCharacters()
        {
            if (characterPrefab == null)
            {
#if UNITY_EDITOR
                characterPrefab = UnityEditor.AssetDatabase.LoadAssetAtPath<GameObject>("Assets/FBX/42.fbx");
#endif
                if (characterPrefab == null)
                {
                    Debug.LogError("[CharacterSpawner] characterPrefab is not assigned! Please assign an FBX model.");
                    return;
                }
            }

            var world = World.DefaultGameObjectInjectionWorld;
            if (world == null)
            {
                Debug.LogError("[CharacterSpawner] DefaultGameObjectInjectionWorld is null!");
                return;
            }

            SpawnCharactersInternal(
                world.EntityManager,
                characterPrefab,
                spawnCount,
                spawnAreaSize,
                spawnInGrid,
                baseHeight,
                moveSpeedMin,
                moveSpeedMax,
                movementBoundsSize,
                transform.position
            );
        }

        /// <summary>
        /// Core spawning implementation used by both MonoBehaviour.Start() and CharacterSpawnerSystem (SubScene).
        /// </summary>
        public static void SpawnCharactersInternal(
            EntityManager entityManager,
            GameObject prefab,
            int count,
            Vector2 spawnArea,
            bool grid,
            float height,
            float minSpeed,
            float maxSpeed,
            Vector2 boundsSize,
            Vector3 centerPos)
        {
            List<SubmeshData> submeshes = CollectSubmeshes(prefab);
            if (submeshes.Count == 0)
            {
                Debug.LogError($"[CharacterSpawner] No meshes found in '{prefab.name}'!");
                return;
            }

            SpawnCharactersFromSubmeshes(
                entityManager,
                submeshes,
                count,
                spawnArea,
                grid,
                height,
                minSpeed,
                maxSpeed,
                boundsSize,
                centerPos
            );
        }

        /// <summary>
        /// Batch-creates 10,000 root entities with movement, and parented child entities for each submesh.
        /// </summary>
        public static void SpawnCharactersFromSubmeshes(
            EntityManager entityManager,
            List<SubmeshData> submeshes,
            int count,
            Vector2 spawnArea,
            bool grid,
            float height,
            float minSpeed,
            float maxSpeed,
            Vector2 boundsSize,
            Vector3 centerPos)
        {
            // 1. Define Archetypes
            var rootArchetype = entityManager.CreateArchetype(
                typeof(LocalTransform),
                typeof(LocalToWorld),
                typeof(CharacterMovementComponent)
            );

            var childArchetype = entityManager.CreateArchetype(
                typeof(Parent),
                typeof(LocalTransform),
                typeof(LocalToWorld),
                typeof(RenderMeshComponent),
                typeof(VisibilityComponent),
                typeof(AABBComponent)
            );

            // 2. Batch Create Root Entities
            using NativeArray<Entity> rootEntities = entityManager.CreateEntity(rootArchetype, count, Allocator.Temp);

            float halfX = spawnArea.x * 0.5f;
            float halfZ = spawnArea.y * 0.5f;
            Vector3 origin = centerPos - new Vector3(halfX, 0, halfZ);

            float halfBoundX = boundsSize.x * 0.5f;
            float halfBoundZ = boundsSize.y * 0.5f;
            float3 boundsMin = new float3(centerPos.x - halfBoundX, centerPos.y + height, centerPos.z - halfBoundZ);
            float3 boundsMax = new float3(centerPos.x + halfBoundX, centerPos.y + height, centerPos.z + halfBoundZ);

            int gridX = Mathf.CeilToInt(Mathf.Sqrt(count));
            int gridZ = Mathf.CeilToInt((float)count / Mathf.Max(1, gridX));
            float stepX = spawnArea.x / Mathf.Max(1, gridX - 1);
            float stepZ = spawnArea.y / Mathf.Max(1, gridZ - 1);

            for (int i = 0; i < count; i++)
            {
                float3 spawnPos;
                if (grid)
                {
                    int gx = i % gridX;
                    int gz = i / gridX;
                    spawnPos = new float3(origin.x + gx * stepX, centerPos.y + height, origin.z + gz * stepZ);
                }
                else
                {
                    float rx = UnityEngine.Random.Range(origin.x, origin.x + spawnArea.x);
                    float rz = UnityEngine.Random.Range(origin.z, origin.z + spawnArea.y);
                    spawnPos = new float3(rx, centerPos.y + height, rz);
                }

                float angle = UnityEngine.Random.Range(0f, Mathf.PI * 2f);
                quaternion rot = quaternion.RotateY(angle);
                float speed = UnityEngine.Random.Range(minSpeed, maxSpeed);
                float3 vel = new float3(math.cos(angle) * speed, 0f, math.sin(angle) * speed);

                entityManager.SetComponentData(rootEntities[i], LocalTransform.FromPositionRotationScale(spawnPos, rot, 1f));
                entityManager.SetComponentData(rootEntities[i], new CharacterMovementComponent
                {
                    Velocity = vel,
                    MoveSpeed = speed,
                    BoundsMin = boundsMin,
                    BoundsMax = boundsMax,
                    DirectionChangeTimer = UnityEngine.Random.Range(1f, 4f),
                    RandomSeed = (uint)(i + 1) * 2654435761u
                });
            }

            // 3. Batch Create Child Submesh Entities
            for (int s = 0; s < submeshes.Count; s++)
            {
                var sub = submeshes[s];
                using NativeArray<Entity> childEntities = entityManager.CreateEntity(childArchetype, count, Allocator.Temp);

                for (int i = 0; i < count; i++)
                {
                    Entity child = childEntities[i];
                    entityManager.SetComponentData(child, new Parent { Value = rootEntities[i] });
                    entityManager.SetComponentData(child, sub.relTransform);
                    entityManager.SetComponentData(child, new RenderMeshComponent
                    {
                        MeshID = sub.meshId,
                        SortKey = sub.sortKey
                    });
                    entityManager.SetComponentData(child, new VisibilityComponent { IsVisible = 1 });
                    entityManager.SetComponentData(child, new AABBComponent
                    {
                        Center = sub.boundsCenter,
                        Extents = sub.boundsExtents
                    });
                }
            }

            int totalEntities = count + (count * submeshes.Count);
            Debug.Log($"[CharacterSpawner] Successfully spawned {count} characters ({totalEntities} total entities with {submeshes.Count} submeshes each)!");
        }

        /// <summary>
        /// Collects all submeshes from the FBX, computes their relative transforms, and uploads them to Vulkan once.
        /// (Called at runtime, never during baking).
        /// </summary>
        public static List<SubmeshData> CollectSubmeshes(GameObject prefab)
        {
            var list = new List<SubmeshData>();
            Transform root = prefab.transform;

            // 1. MeshFilters
            MeshFilter[] meshFilters = prefab.GetComponentsInChildren<MeshFilter>(true);
            foreach (var mf in meshFilters)
            {
                Mesh mesh = mf.sharedMesh;
                if (mesh == null) continue;

                uint meshId = UploadMeshToVulkan(mesh);
                ulong sortKey = ((ulong)meshId) << 16;

                Transform t = mf.transform;
                Vector3 relPos = root.InverseTransformPoint(t.position);
                Quaternion relRot = Quaternion.Inverse(root.rotation) * t.rotation;
                float relScale = root.lossyScale.x != 0 ? t.lossyScale.x / root.lossyScale.x : 1f;

                list.Add(new SubmeshData
                {
                    mesh = mesh,
                    meshId = meshId,
                    sortKey = sortKey,
                    relTransform = LocalTransform.FromPositionRotationScale(relPos, relRot, relScale),
                    boundsCenter = mesh.bounds.center,
                    boundsExtents = mesh.bounds.extents * 1.5f + new Vector3(0.5f, 0.5f, 0.5f)
                });
            }

            // 2. SkinnedMeshRenderers
            SkinnedMeshRenderer[] smrs = prefab.GetComponentsInChildren<SkinnedMeshRenderer>(true);
            foreach (var smr in smrs)
            {
                Mesh mesh = smr.sharedMesh;
                if (mesh == null) continue;

                bool duplicate = false;
                foreach (var existing in list)
                {
                    if (existing.mesh == mesh)
                    {
                        duplicate = true;
                        break;
                    }
                }
                if (duplicate) continue;

                uint meshId = UploadMeshToVulkan(mesh);
                ulong sortKey = ((ulong)meshId) << 16;

                Transform t = smr.transform;
                Vector3 relPos = root.InverseTransformPoint(t.position);
                Quaternion relRot = Quaternion.Inverse(root.rotation) * t.rotation;
                float relScale = root.lossyScale.x != 0 ? t.lossyScale.x / root.lossyScale.x : 1f;

                list.Add(new SubmeshData
                {
                    mesh = mesh,
                    meshId = meshId,
                    sortKey = sortKey,
                    relTransform = LocalTransform.FromPositionRotationScale(relPos, relRot, relScale),
                    boundsCenter = mesh.bounds.center,
                    boundsExtents = mesh.bounds.extents * 1.5f + new Vector3(0.5f, 0.5f, 0.5f)
                });
            }

            return list;
        }

        /// <summary>
        /// Uploads vertex and index buffers to the native Vulkan plugin if not already cached.
        /// </summary>
        public static uint UploadMeshToVulkan(Mesh targetMesh)
        {
            if (s_UploadedMeshCache.TryGetValue(targetMesh, out uint cachedId))
            {
                return cachedId;
            }

            uint meshId = GlobalMeshIdCounter++;

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
                s_UploadedMeshCache[targetMesh] = meshId;
                Debug.Log($"[CharacterSpawner] Uploaded submesh '{targetMesh.name}' to Vulkan (ID: {meshId}, Verts: {vertices.Length}, Tris: {indices.Length / 3})");
            }
            finally
            {
                vertexHandle.Free();
                indexHandle.Free();
            }

            return meshId;
        }

        private void OnDrawGizmosSelected()
        {
            Vector3 center = transform.position + new Vector3(0, baseHeight + 1f, 0);
            
            // Draw Spawn Area (Cyan)
            Gizmos.color = new Color(0f, 1f, 1f, 0.4f);
            Gizmos.DrawWireCube(center, new Vector3(spawnAreaSize.x, 2f, spawnAreaSize.y));

            // Draw Movement Bounds (Green)
            Gizmos.color = new Color(0f, 1f, 0f, 0.6f);
            Gizmos.DrawWireCube(center, new Vector3(movementBoundsSize.x, 4f, movementBoundsSize.y));
        }
    }

    // =========================================================================
    // DOTS SubScene Baking Support
    // =========================================================================

    /// <summary>
    /// Request component baked into the SubScene entity.
    /// Picked up at runtime by CharacterSpawnerSystem.
    /// </summary>
    public struct CharacterSpawnerRequest : IComponentData
    {
        public UnityObjectRef<GameObject> Prefab;
        public int SpawnCount;
        public float2 SpawnAreaSize;
        public byte SpawnInGrid;
        public float BaseHeight;
        public float MoveSpeedMin;
        public float MoveSpeedMax;
        public float2 MovementBoundsSize;
    }

    /// <summary>
    /// Submesh data buffer baked into the entity so mesh references are preserved in SubScenes.
    /// </summary>
    public struct SubmeshBakeData : IBufferElementData
    {
        public UnityObjectRef<Mesh> MeshRef;
        public LocalTransform RelTransform;
        public float3 BoundsCenter;
        public float3 BoundsExtents;
    }

    /// <summary>
    /// Baker that processes CharacterSpawner GameObjects in DOTS SubScenes!
    /// Pure baking logic: NO graphics API or Vulkan calls are made during baking.
    /// </summary>
    public class CharacterSpawnerBaker : Baker<CharacterSpawner>
    {
        public override void Bake(CharacterSpawner authoring)
        {
            if (authoring.characterPrefab == null)
            {
#if UNITY_EDITOR
                authoring.characterPrefab = UnityEditor.AssetDatabase.LoadAssetAtPath<GameObject>("Assets/FBX/42.fbx");
#endif
            }

            if (authoring.characterPrefab == null) return;

            Entity entity = GetEntity(TransformUsageFlags.Dynamic);

            AddComponent(entity, new CharacterSpawnerRequest
            {
                Prefab = authoring.characterPrefab,
                SpawnCount = authoring.spawnCount,
                SpawnAreaSize = authoring.spawnAreaSize,
                SpawnInGrid = authoring.spawnInGrid ? (byte)1 : (byte)0,
                BaseHeight = authoring.baseHeight,
                MoveSpeedMin = authoring.moveSpeedMin,
                MoveSpeedMax = authoring.moveSpeedMax,
                MovementBoundsSize = authoring.movementBoundsSize
            });

            // Pure metadata extraction (NO Vulkan calls!)
            var buffer = AddBuffer<SubmeshBakeData>(entity);
            Transform root = authoring.characterPrefab.transform;

            MeshFilter[] meshFilters = authoring.characterPrefab.GetComponentsInChildren<MeshFilter>(true);
            foreach (var mf in meshFilters)
            {
                if (mf.sharedMesh == null) continue;
                Transform t = mf.transform;
                Vector3 relPos = root.InverseTransformPoint(t.position);
                Quaternion relRot = Quaternion.Inverse(root.rotation) * t.rotation;
                float relScale = root.lossyScale.x != 0 ? t.lossyScale.x / root.lossyScale.x : 1f;

                buffer.Add(new SubmeshBakeData
                {
                    MeshRef = mf.sharedMesh,
                    RelTransform = LocalTransform.FromPositionRotationScale(relPos, relRot, relScale),
                    BoundsCenter = mf.sharedMesh.bounds.center,
                    BoundsExtents = mf.sharedMesh.bounds.extents * 1.5f + new Vector3(0.5f, 0.5f, 0.5f)
                });
            }

            SkinnedMeshRenderer[] smrs = authoring.characterPrefab.GetComponentsInChildren<SkinnedMeshRenderer>(true);
            foreach (var smr in smrs)
            {
                if (smr.sharedMesh == null) continue;
                Transform t = smr.transform;
                Vector3 relPos = root.InverseTransformPoint(t.position);
                Quaternion relRot = Quaternion.Inverse(root.rotation) * t.rotation;
                float relScale = root.lossyScale.x != 0 ? t.lossyScale.x / root.lossyScale.x : 1f;

                buffer.Add(new SubmeshBakeData
                {
                    MeshRef = smr.sharedMesh,
                    RelTransform = LocalTransform.FromPositionRotationScale(relPos, relRot, relScale),
                    BoundsCenter = smr.sharedMesh.bounds.center,
                    BoundsExtents = smr.sharedMesh.bounds.extents * 1.5f + new Vector3(0.5f, 0.5f, 0.5f)
                });
            }
        }
    }
}
