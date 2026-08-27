using System.Runtime.InteropServices;
using Unity.Collections;
using System;

namespace Endfield.Rendering
{
    public static class VulkanPluginWrapper
    {
        private const string pluginName = "MiniEndfieldVulkanPlugin";

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate void DebugLogCallback([MarshalAs(UnmanagedType.LPStr)] string message);

        [DllImport(pluginName)]
        public static extern void SetDebugLogCallback(DebugLogCallback callback);

        // Keep a reference to prevent garbage collection
        private static DebugLogCallback _debugCallback = new DebugLogCallback(LogMessageFromVulkan);

        [AOT.MonoPInvokeCallback(typeof(DebugLogCallback))]
        private static void LogMessageFromVulkan(string message)
        {
            UnityEngine.Debug.Log(message);
        }

        [DllImport("user32.dll")]
        private static extern IntPtr GetActiveWindow();

        public static void InitializeWithDebug()
        {
            SetDebugLogCallback(_debugCallback);
            IntPtr windowHandle = GetActiveWindow();
            InitializeVulkanBackend(windowHandle);
        }

        [DllImport(pluginName)]
        public static extern void InitializeVulkanBackend(IntPtr windowHandle);

        [DllImport(pluginName)]
        public static extern void ShutdownVulkanBackend();

        [DllImport(pluginName)]
        public static extern void SetupRenderGraph();

        [DllImport(pluginName)]
        public static extern void BeginFrame();

        [DllImport(pluginName)]
        public static extern void SubmitRenderBatch(IntPtr batchData, int instanceCount);

        [DllImport(pluginName)]
        public static extern void EndFrame();
    }
}

