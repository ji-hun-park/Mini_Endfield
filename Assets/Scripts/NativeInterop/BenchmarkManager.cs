using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using Unity.Entities;
using Endfield.Rendering;
using Endfield.ECS;
using Endfield.ECS.Systems;

namespace Endfield.NativeInterop
{
    public class BenchmarkManager : MonoBehaviour
    {
        public static BenchmarkManager Instance { get; private set; }

        [Header("Culling Settings")]
        [SerializeField] private bool m_EnableFrustumCulling = true;
        [SerializeField] private bool m_EnableOcclusionCulling = true;

        [Header("Automated Benchmark Configuration")]
        [SerializeField] private int[] m_BenchmarkTiers = new int[] { 1000, 10000, 50000, 100000, 500000 };
        [SerializeField] private float m_BenchmarkSpreadRadius = 300.0f;
        [SerializeField] private int m_WarmupFrames = 15;
        [SerializeField] private int m_SampleFrames = 45;

        public bool IsBenchmarkRunning { get; private set; } = false;
        public string BenchmarkStatusMessage { get; private set; } = "Idle";
        public float BenchmarkProgress { get; private set; } = 0.0f;

        // Live stats
        private VulkanPluginWrapper.NativeBenchmarkStats m_LatestNativeStats;
        private readonly Queue<float> m_FrameTimeBuffer = new Queue<float>();
        private const int MAX_FRAME_SAMPLES = 60;

        public float CurrentFps { get; private set; }
        public float AverageFps { get; private set; }
        public float OnePercentLowFps { get; private set; }
        public float MinFps { get; private set; }
        public float MaxFps { get; private set; }

        public VulkanPluginWrapper.NativeBenchmarkStats LatestNativeStats => m_LatestNativeStats;

        public bool EnableFrustumCulling
        {
            get => m_EnableFrustumCulling;
            set
            {
                m_EnableFrustumCulling = value;
                SoftwareCullingSystem.EnableCulling = value;
                VulkanPluginWrapper.SetBenchmarkCullingOptions(value, m_EnableOcclusionCulling);
            }
        }

        public bool EnableOcclusionCulling
        {
            get => m_EnableOcclusionCulling;
            set
            {
                m_EnableOcclusionCulling = value;
                VulkanPluginWrapper.SetBenchmarkCullingOptions(m_EnableFrustumCulling, value);
            }
        }

        public event Action<string> OnBenchmarkCompleted;

        [RuntimeInitializeOnLoadMethod(RuntimeInitializeLoadType.AfterSceneLoad)]
        private static void AutoInitialize()
        {
            if (Instance == null)
            {
                var go = new GameObject("[Endfield Benchmark Controller]");
                go.AddComponent<BenchmarkManager>();
                go.AddComponent<BenchmarkHUD>();
            }
        }

        private void Awake()
        {
            if (Instance != null && Instance != this)
            {
                Destroy(gameObject);
                return;
            }
            Instance = this;
            DontDestroyOnLoad(gameObject);
        }

        private void Start()
        {
            EnableFrustumCulling = m_EnableFrustumCulling;
            EnableOcclusionCulling = m_EnableOcclusionCulling;
        }

        private void Update()
        {
            // 1. Calculate live Unity FPS & frame times
            float dt = Time.unscaledDeltaTime;
            if (dt > 0.0001f)
            {
                CurrentFps = 1.0f / dt;
                m_FrameTimeBuffer.Enqueue(CurrentFps);
                if (m_FrameTimeBuffer.Count > MAX_FRAME_SAMPLES)
                {
                    m_FrameTimeBuffer.Dequeue();
                }

                CalculateFpsMetrics();
            }

            // 2. Poll latest Native C++ stats
            try
            {
                VulkanPluginWrapper.GetLatestBenchmarkStats(out m_LatestNativeStats);
            }
            catch
            {
                // Native plugin might not be initialized yet
            }
        }

        private void CalculateFpsMetrics()
        {
            if (m_FrameTimeBuffer.Count == 0) return;

            float sum = 0.0f;
            float min = float.MaxValue;
            float max = float.MinValue;
            List<float> sortedSamples = new List<float>(m_FrameTimeBuffer.Count);

            foreach (float fps in m_FrameTimeBuffer)
            {
                sum += fps;
                if (fps < min) min = fps;
                if (fps > max) max = fps;
                sortedSamples.Add(fps);
            }

            AverageFps = sum / m_FrameTimeBuffer.Count;
            MinFps = min;
            MaxFps = max;

            sortedSamples.Sort();
            int onePercentIndex = Mathf.Max(0, Mathf.FloorToInt(sortedSamples.Count * 0.01f));
            OnePercentLowFps = sortedSamples[onePercentIndex];
        }

        public void ClearAllCharacters()
        {
            var world = World.DefaultGameObjectInjectionWorld;
            if (world == null) return;
            var em = world.EntityManager;

            var childQuery = em.CreateEntityQuery(ComponentType.ReadOnly<RenderMeshComponent>());
            em.DestroyEntity(childQuery);
            childQuery.Dispose();

            var rootQuery = em.CreateEntityQuery(ComponentType.ReadOnly<CharacterMovementComponent>());
            em.DestroyEntity(rootQuery);
            rootQuery.Dispose();
        }

        public void SpawnExtraCharacters(int count, float spreadRadius = -1f)
        {
            var world = World.DefaultGameObjectInjectionWorld;
            if (world == null)
            {
                Debug.LogWarning("[BenchmarkManager] DefaultGameObjectInjectionWorld is null!");
                return;
            }

            GameObject prefab = null;
#if UNITY_EDITOR
            prefab = UnityEditor.AssetDatabase.LoadAssetAtPath<GameObject>("Assets/FBX/42.fbx");
#endif
            if (prefab == null)
            {
                Debug.LogError("[BenchmarkManager] Cannot load Assets/FBX/42.fbx to spawn extra characters!");
                return;
            }

            float spread = spreadRadius > 0f ? spreadRadius : m_BenchmarkSpreadRadius;

            CharacterSpawner.SpawnCharactersInternal(
                world.EntityManager,
                prefab,
                count,
                new Vector2(spread, spread),
                false,
                0f,
                3f,
                8f,
                new Vector2(spread * 1.1f, spread * 1.1f),
                Vector3.zero
            );
        }

        public void StartMultiTierBenchmark()
        {
            if (IsBenchmarkRunning) return;
            StartCoroutine(RunMultiTierBenchmarkRoutine());
        }

        private IEnumerator RunMultiTierBenchmarkRoutine()
        {
            IsBenchmarkRunning = true;
            BenchmarkStatusMessage = "Starting Multi-Tier Benchmark Suite...";
            List<BenchmarkTierResult> results = new List<BenchmarkTierResult>();

            EnableFrustumCulling = true;

            for (int t = 0; t < m_BenchmarkTiers.Length; t++)
            {
                int count = m_BenchmarkTiers[t];
                string tierName = count >= 1000 ? $"{count / 1000}K Tier" : $"{count} Tier";
                BenchmarkStatusMessage = $"Running Tier {tierName} ({count:N0} characters @ spread {m_BenchmarkSpreadRadius:F0}m)...";
                BenchmarkProgress = (float)t / m_BenchmarkTiers.Length;

                // 1. Clear previous characters
                ClearAllCharacters();
                yield return null;

                // 2. Spawn instances for this tier
                SpawnExtraCharacters(count, m_BenchmarkSpreadRadius);
                yield return null;

                // 3. Sample
                yield return StartCoroutine(SampleBenchmarkTier(tierName, results));
            }

            // Restore initial 10K characters with default 5000m spread
            ClearAllCharacters();
            yield return null;
            SpawnExtraCharacters(10000, 5000f);

            // Generate and save report
            BenchmarkStatusMessage = "Compiling Multi-Tier Markdown Report...";
            string reportMarkdown = BenchmarkReportGenerator.GenerateMarkdownReport("Multi-Tier Density Benchmark (Spread 300m)", results);
            string savedPath = BenchmarkReportGenerator.SaveReportToFile(reportMarkdown);

            BenchmarkStatusMessage = "Multi-Tier Benchmark Completed!";
            IsBenchmarkRunning = false;
            OnBenchmarkCompleted?.Invoke(savedPath);
        }

        public void StartAutomatedBenchmark()
        {
            if (IsBenchmarkRunning) return;
            StartCoroutine(RunAutomatedBenchmarkRoutine());
        }

        private IEnumerator RunAutomatedBenchmarkRoutine()
        {
            IsBenchmarkRunning = true;
            BenchmarkStatusMessage = "Starting Benchmark Suite...";
            List<BenchmarkTierResult> results = new List<BenchmarkTierResult>();

            // Mode 1: Benchmark with Culling ENABLED
            EnableFrustumCulling = true;
            BenchmarkStatusMessage = "Evaluating with Frustum Culling ON...";
            yield return StartCoroutine(SampleBenchmarkTier("Frustum Culling ON (Default)", results));

            // Mode 2: Benchmark with Culling DISABLED (Full Stress Test)
            EnableFrustumCulling = false;
            BenchmarkStatusMessage = "Evaluating with Frustum Culling OFF (Stress Test)...";
            yield return StartCoroutine(SampleBenchmarkTier("Frustum Culling OFF (No Culling)", results));

            // Restore Culling
            EnableFrustumCulling = true;

            // Generate and save report
            BenchmarkStatusMessage = "Compiling Markdown Report...";
            string reportMarkdown = BenchmarkReportGenerator.GenerateMarkdownReport("Automated Runtime Benchmark", results);
            string savedPath = BenchmarkReportGenerator.SaveReportToFile(reportMarkdown);

            BenchmarkStatusMessage = "Benchmark Completed!";
            IsBenchmarkRunning = false;
            OnBenchmarkCompleted?.Invoke(savedPath);
        }

        private IEnumerator SampleBenchmarkTier(string tierName, List<BenchmarkTierResult> results)
        {
            // Warmup
            for (int i = 0; i < m_WarmupFrames; i++)
            {
                BenchmarkProgress = (float)i / (m_WarmupFrames + m_SampleFrames);
                yield return null;
            }

            // Sampling
            float totalFps = 0.0f;
            float totalFrameTime = 0.0f;
            float totalCullMs = 0.0f;
            float totalPackMs = 0.0f;
            float totalSortMs = 0.0f;
            float totalBatchMs = 0.0f;
            uint totalVisible = 0;
            uint totalCand = 0;

            for (int i = 0; i < m_SampleFrames; i++)
            {
                BenchmarkProgress = (float)(m_WarmupFrames + i) / (m_WarmupFrames + m_SampleFrames);
                float dt = Time.unscaledDeltaTime;
                totalFps += (dt > 0.0001f) ? (1.0f / dt) : 60f;
                totalFrameTime += dt * 1000.0f;

                totalCullMs += SoftwareCullingSystem.LastCullingTimeMs;
                totalPackMs += RenderSubmissionSystem.LastPackAndSubmitTimeMs;
                totalSortMs += m_LatestNativeStats.sortingTimeMs;
                totalBatchMs += m_LatestNativeStats.batchingTimeMs;
                totalVisible += (uint)RenderSubmissionSystem.LastVisibleInstanceCount;
                totalCand += (uint)RenderSubmissionSystem.LastTotalCandidateCount;

                yield return null;
            }

            int count = Mathf.Max(1, m_SampleFrames);
            uint avgCand = totalCand / (uint)count;
            uint avgVis = totalVisible / (uint)count;
            float cullRatio = avgCand > 0 ? ((float)(avgCand - avgVis) / avgCand) * 100.0f : 0f;

            // Estimate character count: candidate entities / 20 (or 10)
            int charCount = (int)(avgCand / 20);

            var avgNativeStats = new VulkanPluginWrapper.NativeBenchmarkStats
            {
                totalInstances = avgCand,
                visibleInstances = avgVis,
                culledFrustum = avgCand - avgVis,
                sortingTimeMs = totalSortMs / count,
                batchingTimeMs = totalBatchMs / count,
                totalNativeFrameTimeMs = (totalSortMs + totalBatchMs) / count
            };

            results.Add(new BenchmarkTierResult
            {
                tierName = tierName,
                characterCount = charCount,
                entityCount = (int)avgCand,
                visibleEntities = (int)avgVis,
                cullingRatio = cullRatio,
                unityFps = totalFps / count,
                unityFrameTimeMs = totalFrameTime / count,
                dotsCullingTimeMs = totalCullMs / count,
                dotsPackTimeMs = totalPackMs / count,
                nativeStats = avgNativeStats
            });
        }
    }
}

