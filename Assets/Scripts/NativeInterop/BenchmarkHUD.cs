using UnityEngine;
using Endfield.ECS.Systems;

namespace Endfield.NativeInterop
{
    [RequireComponent(typeof(BenchmarkManager))]
    public class BenchmarkHUD : MonoBehaviour
    {
        [Header("HUD Settings")]
        [SerializeField] private KeyCode m_ToggleKey = KeyCode.F1;
        [SerializeField] private KeyCode m_AlternateToggleKey = KeyCode.B;
        [SerializeField] private bool m_ShowHUD = true;

        private BenchmarkManager m_Manager;
        private GUIStyle m_BoxStyle;
        private GUIStyle m_HeaderStyle;
        private GUIStyle m_LabelStyle;
        private GUIStyle m_MetricStyle;
        private GUIStyle m_HighlightStyle;
        private GUIStyle m_PassStyle;
        private GUIStyle m_WarnStyle;
        private GUIStyle m_ButtonStyle;
        private bool m_StylesInitialized = false;

        private string m_LastReportPath = "";

        private void Start()
        {
            m_Manager = GetComponent<BenchmarkManager>();
            m_Manager.OnBenchmarkCompleted += (path) => m_LastReportPath = path;
        }

        private void Update()
        {
            if (Input.GetKeyDown(m_ToggleKey) || Input.GetKeyDown(m_AlternateToggleKey))
            {
                m_ShowHUD = !m_ShowHUD;
            }
        }

        private void InitializeStyles()
        {
            if (m_StylesInitialized) return;

            m_BoxStyle = new GUIStyle(GUI.skin.box);
            m_BoxStyle.normal.background = MakeTex(2, 2, new Color(0.06f, 0.07f, 0.09f, 0.94f));

            m_HeaderStyle = new GUIStyle(GUI.skin.label)
            {
                fontSize = 13,
                fontStyle = FontStyle.Bold,
                alignment = TextAnchor.MiddleLeft
            };
            m_HeaderStyle.normal.textColor = new Color(0.35f, 0.75f, 1.0f);

            m_LabelStyle = new GUIStyle(GUI.skin.label)
            {
                fontSize = 11
            };
            m_LabelStyle.normal.textColor = new Color(0.85f, 0.85f, 0.85f);

            m_MetricStyle = new GUIStyle(GUI.skin.label)
            {
                fontSize = 11,
                fontStyle = FontStyle.Bold
            };
            m_MetricStyle.normal.textColor = new Color(0.4f, 0.95f, 0.5f);

            m_HighlightStyle = new GUIStyle(GUI.skin.label)
            {
                fontSize = 11,
                fontStyle = FontStyle.Bold
            };
            m_HighlightStyle.normal.textColor = new Color(1.0f, 0.85f, 0.3f);

            m_PassStyle = new GUIStyle(GUI.skin.label)
            {
                fontSize = 11,
                fontStyle = FontStyle.Bold
            };
            m_PassStyle.normal.textColor = new Color(0.3f, 1.0f, 0.4f);

            m_WarnStyle = new GUIStyle(GUI.skin.label)
            {
                fontSize = 11,
                fontStyle = FontStyle.Bold
            };
            m_WarnStyle.normal.textColor = new Color(1.0f, 0.4f, 0.3f);

            m_ButtonStyle = new GUIStyle(GUI.skin.button)
            {
                fontSize = 11,
                fontStyle = FontStyle.Bold
            };

            m_StylesInitialized = true;
        }

        private Texture2D MakeTex(int width, int height, Color col)
        {
            Color[] pix = new Color[width * height];
            for (int i = 0; i < pix.Length; ++i) pix[i] = col;
            Texture2D result = new Texture2D(width, height);
            result.SetPixels(pix);
            result.Apply();
            return result;
        }

        private void OnGUI()
        {
            if (!m_ShowHUD)
            {
                InitializeStyles();
                if (GUI.Button(new Rect(15, 15, 190, 26), $"Show Benchmark ({m_ToggleKey}/{m_AlternateToggleKey})", m_ButtonStyle))
                {
                    m_ShowHUD = true;
                }
                return;
            }

            InitializeStyles();

            float panelWidth = 380f;
            float panelHeight = m_Manager.IsBenchmarkRunning ? 520f : 470f;
            Rect panelRect = new Rect(15, 15, panelWidth, panelHeight);

            GUI.Box(panelRect, GUIContent.none, m_BoxStyle);

            GUILayout.BeginArea(new Rect(panelRect.x + 12, panelRect.y + 10, panelRect.width - 24, panelRect.height - 20));

            // Header
            GUILayout.BeginHorizontal();
            GUILayout.Label("ARKKNIGHTS: ENDFIELD BENCHMARK", m_HeaderStyle);
            if (GUILayout.Button("X", m_ButtonStyle, GUILayout.Width(24), GUILayout.Height(20)))
            {
                m_ShowHUD = false;
            }
            GUILayout.EndHorizontal();

            GUILayout.Space(4);

            // 1. Live Frame Rates
            GUILayout.BeginHorizontal();
            GUILayout.Label($"FPS: {m_Manager.CurrentFps:F1}", m_HighlightStyle);
            GUILayout.Label($"Avg: {m_Manager.AverageFps:F1}", m_LabelStyle);
            GUILayout.Label($"1% Low: {m_Manager.OnePercentLowFps:F1}", m_MetricStyle);
            GUILayout.Label($"Min: {m_Manager.MinFps:F0} / Max: {m_Manager.MaxFps:F0}", m_LabelStyle);
            GUILayout.EndHorizontal();

            GUILayout.Space(2);
            DrawSeparator();

            // 2. Candidate & Visible Entities
            int totalCand = RenderSubmissionSystem.LastTotalCandidateCount;
            int visible = RenderSubmissionSystem.LastVisibleInstanceCount;
            int culled = RenderSubmissionSystem.LastCulledCount;
            float cullRatio = RenderSubmissionSystem.LastCullingRatio;

            GUILayout.Label("ENTITY & CULLING STATS", m_HeaderStyle);
            GUILayout.BeginHorizontal();
            GUILayout.Label($"Total: {totalCand:N0}", m_LabelStyle);
            GUILayout.Label($"Visible: {visible:N0}", m_MetricStyle);
            GUILayout.Label($"Culled: {culled:N0} ({cullRatio:F1}%)", m_HighlightStyle);
            GUILayout.EndHorizontal();

            GUILayout.Space(2);
            DrawSeparator();

            // 3. Pipeline Timing Breakdown
            var native = m_Manager.LatestNativeStats;
            float sortBatchSum = native.sortingTimeMs + native.batchingTimeMs;

            GUILayout.Label("PIPELINE STAGE BREAKDOWN", m_HeaderStyle);
            DrawTimingRow("DOTS Frustum Cull", $"{SoftwareCullingSystem.LastCullingTimeMs:F3} ms");
            DrawTimingRow("DOTS Instance Pack", $"{RenderSubmissionSystem.LastPackAndSubmitTimeMs:F3} ms");
            DrawTimingRow("Native 64-bit Sort", $"{native.sortingTimeMs:F3} ms");
            DrawTimingRow("Native Batch & Draw", $"{native.batchingTimeMs:F3} ms");

            // Endfield Target Comparison (< 1.0 ms on PC)
            GUILayout.BeginHorizontal();
            GUILayout.Label("  Sort + Batch Total:", m_LabelStyle);
            GUIStyle targetStyle = (sortBatchSum <= 1.0f) ? m_PassStyle : m_WarnStyle;
            string targetStatus = (sortBatchSum <= 1.0f) ? "(PASS < 1.0ms)" : "(TARGET: 1.0ms)";
            GUILayout.Label($"{sortBatchSum:F3} ms {targetStatus}", targetStyle, GUILayout.Width(170));
            GUILayout.EndHorizontal();

            GUILayout.Space(4);
            DrawSeparator();

            // 4. Interactive Controls
            GUILayout.Label("INTERACTIVE CONTROLS", m_HeaderStyle);

            // Culling Toggle
            bool cullToggle = GUILayout.Toggle(m_Manager.EnableFrustumCulling, " Enable Frustum Culling");
            if (cullToggle != m_Manager.EnableFrustumCulling)
            {
                m_Manager.EnableFrustumCulling = cullToggle;
            }

            // Quick Spawner
            GUILayout.BeginHorizontal();
            GUILayout.Label("Add Characters:", m_LabelStyle, GUILayout.Width(100));
            if (GUILayout.Button("+1,000", m_ButtonStyle))
            {
                m_Manager.SpawnExtraCharacters(1000);
            }
            if (GUILayout.Button("+5,000", m_ButtonStyle))
            {
                m_Manager.SpawnExtraCharacters(5000);
            }
            if (GUILayout.Button("+10,000", m_ButtonStyle))
            {
                m_Manager.SpawnExtraCharacters(10000);
            }
            GUILayout.EndHorizontal();

            GUILayout.Space(6);

            // Automated Suite
            if (m_Manager.IsBenchmarkRunning)
            {
                GUI.enabled = false;
                GUILayout.Button($"Running: {m_Manager.BenchmarkStatusMessage} ({m_Manager.BenchmarkProgress * 100:F0}%)", m_ButtonStyle, GUILayout.Height(28));
                GUI.enabled = true;
            }
            else
            {
                if (GUILayout.Button("RUN AUTOMATED BENCHMARK SUITE", m_ButtonStyle, GUILayout.Height(28)))
                {
                    m_Manager.StartAutomatedBenchmark();
                }
            }

            if (!string.IsNullOrEmpty(m_LastReportPath))
            {
                GUILayout.Space(4);
                GUILayout.Label($"Report Saved:\n{m_LastReportPath}", m_LabelStyle);
            }

            GUILayout.EndArea();
        }

        private void DrawTimingRow(string label, string timeStr)
        {
            GUILayout.BeginHorizontal();
            GUILayout.Label($"  {label}:", m_LabelStyle);
            GUILayout.Label(timeStr, m_MetricStyle, GUILayout.Width(100));
            GUILayout.EndHorizontal();
        }

        private void DrawSeparator()
        {
            GUILayout.Box(GUIContent.none, GUILayout.ExpandWidth(true), GUILayout.Height(1));
        }
    }
}

