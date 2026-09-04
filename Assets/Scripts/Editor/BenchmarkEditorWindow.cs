using System;
using System.IO;
using UnityEngine;
using UnityEditor;
using Endfield.Rendering;
using Endfield.NativeInterop;
using Endfield.ECS.Systems;

namespace Endfield.EditorTools
{
    public class BenchmarkEditorWindow : EditorWindow
    {
        private int m_SelectedTab = 0;
        private readonly string[] m_TabNames = new string[] { "Headless C++ Benchmark", "Live In-Game Benchmark", "Saved Reports" };

        // Headless C++ Benchmark Settings
        private int m_HeadlessInstanceCount = 100000;
        private int m_HeadlessIterations = 10;
        private bool m_HeadlessRan = false;
        private VulkanPluginWrapper.NativeBenchmarkStats m_HeadlessResult;

        // Saved Reports
        private string[] m_ReportFiles = new string[0];
        private int m_SelectedReportIndex = 0;
        private string m_SelectedReportContent = "";
        private Vector2 m_ReportScrollPos;

        [MenuItem("Endfield/Benchmark Runner", false, 10)]
        public static void Open()
        {
            var window = GetWindow<BenchmarkEditorWindow>("Benchmark Runner");
            window.minSize = new Vector2(500, 450);
            window.Show();
        }

        private void OnEnable()
        {
            RefreshReportList();
        }

        private void OnGUI()
        {
            EditorGUILayout.Space(6);
            m_SelectedTab = GUILayout.Toolbar(m_SelectedTab, m_TabNames, GUILayout.Height(26));
            EditorGUILayout.Space(10);

            switch (m_SelectedTab)
            {
                case 0:
                    DrawHeadlessBenchmarkTab();
                    break;
                case 1:
                    DrawLiveBenchmarkTab();
                    break;
                case 2:
                    DrawSavedReportsTab();
                    break;
            }
        }

        private void DrawHeadlessBenchmarkTab()
        {
            EditorGUILayout.LabelField("Native C++ Algorithmic Benchmark (No GPU Window Required)", EditorStyles.boldLabel);
            EditorGUILayout.HelpBox(
                "Benchmarks pure C++ algorithms (64-bit SortKey quicksort, redundant bind elimination, dynamic batch build) on scale up to 600,000 instances.",
                MessageType.Info
            );

            EditorGUILayout.Space(6);
            m_HeadlessInstanceCount = EditorGUILayout.IntField("Instance Count:", m_HeadlessInstanceCount);
            m_HeadlessIterations = EditorGUILayout.IntSlider("Iterations:", m_HeadlessIterations, 1, 50);

            EditorGUILayout.BeginHorizontal();
            EditorGUILayout.PrefixLabel("Quick Presets:");
            if (GUILayout.Button("10K")) m_HeadlessInstanceCount = 10000;
            if (GUILayout.Button("50K")) m_HeadlessInstanceCount = 50000;
            if (GUILayout.Button("100K")) m_HeadlessInstanceCount = 100000;
            if (GUILayout.Button("300K")) m_HeadlessInstanceCount = 300000;
            if (GUILayout.Button("600K (Max)")) m_HeadlessInstanceCount = 600000;
            EditorGUILayout.EndHorizontal();

            EditorGUILayout.Space(12);

            GUI.backgroundColor = new Color(0.3f, 0.7f, 1.0f);
            if (GUILayout.Button($"RUN C++ HEADLESS BENCHMARK ({m_HeadlessInstanceCount:N0} instances)", GUILayout.Height(34)))
            {
                RunHeadlessBenchmark();
            }
            GUI.backgroundColor = Color.white;

            if (m_HeadlessRan)
            {
                EditorGUILayout.Space(12);
                EditorGUILayout.LabelField("Benchmark Results:", EditorStyles.boldLabel);

                float sortBatchSum = m_HeadlessResult.sortingTimeMs + m_HeadlessResult.batchingTimeMs;
                bool passedPC = sortBatchSum <= 1.0f;

                EditorGUILayout.BeginVertical(EditorStyles.helpBox);
                EditorGUILayout.LabelField($"Tested Instances: {m_HeadlessResult.totalInstances:N0}");
                EditorGUILayout.LabelField($"Average Iterations: {m_HeadlessIterations}");
                EditorGUILayout.Space(4);
                EditorGUILayout.LabelField($"64-bit SortKey Sort Time: {m_HeadlessResult.sortingTimeMs:F3} ms");
                EditorGUILayout.LabelField($"Dynamic Batching Build Time: {m_HeadlessResult.batchingTimeMs:F3} ms");
                EditorGUILayout.Space(4);

                GUI.color = passedPC ? new Color(0.2f, 0.9f, 0.3f) : new Color(1.0f, 0.4f, 0.2f);
                EditorGUILayout.LabelField($"Total Sort + Batch: {sortBatchSum:F3} ms {(passedPC ? "✔ PASSES Endfield PC Target (< 1.0ms)" : "⚠ Above 1.0ms")}", EditorStyles.boldLabel);
                GUI.color = Color.white;

                float theoreticalFps = sortBatchSum > 0.0001f ? (1000.0f / sortBatchSum) : 9999f;
                EditorGUILayout.LabelField($"Theoretical Max Algorithm Throughput: {theoreticalFps:F0} FPS");
                EditorGUILayout.EndVertical();
            }
        }

        private void RunHeadlessBenchmark()
        {
            try
            {
                VulkanPluginWrapper.RunNativeHeadlessBenchmark(m_HeadlessInstanceCount, m_HeadlessIterations, out m_HeadlessResult);
                m_HeadlessRan = true;
            }
            catch (Exception ex)
            {
                EditorUtility.DisplayDialog("Benchmark Error", $"Failed to run headless benchmark: {ex.Message}", "OK");
            }
        }

        private void DrawLiveBenchmarkTab()
        {
            EditorGUILayout.LabelField("Live In-Game PlayMode Benchmark", EditorStyles.boldLabel);

            if (!Application.isPlaying)
            {
                EditorGUILayout.HelpBox("Live In-Game Benchmark requires Unity to be in Play Mode. Press Play to start!", MessageType.Warning);
                if (GUILayout.Button("Enter Play Mode", GUILayout.Height(30)))
                {
                    EditorApplication.isPlaying = true;
                }
                return;
            }

            var manager = BenchmarkManager.Instance;
            if (manager == null)
            {
                EditorGUILayout.HelpBox("BenchmarkManager instance not found in scene.", MessageType.Error);
                return;
            }

            EditorGUILayout.BeginVertical(EditorStyles.helpBox);
            EditorGUILayout.LabelField($"FPS: {manager.CurrentFps:F1} (Avg: {manager.AverageFps:F1}, 1% Low: {manager.OnePercentLowFps:F1})", EditorStyles.boldLabel);
            EditorGUILayout.LabelField($"Total Candidate Entities: {RenderSubmissionSystem.LastTotalCandidateCount:N0}");
            EditorGUILayout.LabelField($"Visible Rendered Entities: {RenderSubmissionSystem.LastVisibleInstanceCount:N0}");
            EditorGUILayout.LabelField($"Frustum Culled: {RenderSubmissionSystem.LastCulledCount:N0} ({RenderSubmissionSystem.LastCullingRatio:F1}%)");
            EditorGUILayout.EndVertical();

            EditorGUILayout.Space(8);

            // Controls
            manager.EnableFrustumCulling = EditorGUILayout.Toggle("Enable Frustum Culling", manager.EnableFrustumCulling);

            EditorGUILayout.Space(8);

            if (manager.IsBenchmarkRunning)
            {
                GUI.enabled = false;
                GUILayout.Button($"Running: {manager.BenchmarkStatusMessage} ({manager.BenchmarkProgress * 100:F0}%)", GUILayout.Height(32));
                GUI.enabled = true;
            }
            else
            {
                GUI.backgroundColor = new Color(0.4f, 0.9f, 0.4f);
                if (GUILayout.Button("RUN AUTOMATED BENCHMARK SUITE", GUILayout.Height(32)))
                {
                    manager.StartAutomatedBenchmark();
                }
                GUI.backgroundColor = Color.white;
            }
        }

        private void DrawSavedReportsTab()
        {
            EditorGUILayout.LabelField("Generated Benchmark Reports", EditorStyles.boldLabel);

            EditorGUILayout.BeginHorizontal();
            if (GUILayout.Button("Refresh Reports List", GUILayout.Width(150)))
            {
                RefreshReportList();
            }
            if (GUILayout.Button("Open Reports Folder", GUILayout.Width(150)))
            {
                string dir = Path.Combine(Application.dataPath, "../docs/benchmark_reports");
                if (!Directory.Exists(dir)) Directory.CreateDirectory(dir);
                EditorUtility.RevealInFinder(dir);
            }
            EditorGUILayout.EndHorizontal();

            EditorGUILayout.Space(6);

            if (m_ReportFiles.Length == 0)
            {
                EditorGUILayout.HelpBox("No benchmark reports found yet. Run an automated benchmark to generate one!", MessageType.Info);
                return;
            }

            string[] displayNames = new string[m_ReportFiles.Length];
            for (int i = 0; i < m_ReportFiles.Length; i++)
            {
                displayNames[i] = Path.GetFileName(m_ReportFiles[i]);
            }

            int newIndex = EditorGUILayout.Popup("Select Report:", m_SelectedReportIndex, displayNames);
            if (newIndex != m_SelectedReportIndex || string.IsNullOrEmpty(m_SelectedReportContent))
            {
                m_SelectedReportIndex = newIndex;
                LoadSelectedReport();
            }

            EditorGUILayout.Space(6);
            m_ReportScrollPos = EditorGUILayout.BeginScrollView(m_ReportScrollPos, EditorStyles.helpBox);
            EditorGUILayout.TextArea(m_SelectedReportContent, EditorStyles.wordWrappedLabel);
            EditorGUILayout.EndScrollView();
        }

        private void RefreshReportList()
        {
            string dir = Path.Combine(Application.dataPath, "../docs/benchmark_reports");
            if (Directory.Exists(dir))
            {
                m_ReportFiles = Directory.GetFiles(dir, "*.md");
                Array.Sort(m_ReportFiles);
                Array.Reverse(m_ReportFiles); // newest first
            }
            else
            {
                m_ReportFiles = new string[0];
            }

            if (m_ReportFiles.Length > 0)
            {
                m_SelectedReportIndex = Mathf.Clamp(m_SelectedReportIndex, 0, m_ReportFiles.Length - 1);
                LoadSelectedReport();
            }
            else
            {
                m_SelectedReportContent = "";
            }
        }

        private void LoadSelectedReport()
        {
            if (m_ReportFiles.Length > m_SelectedReportIndex && File.Exists(m_ReportFiles[m_SelectedReportIndex]))
            {
                m_SelectedReportContent = File.ReadAllText(m_ReportFiles[m_SelectedReportIndex]);
            }
        }
    }
}

