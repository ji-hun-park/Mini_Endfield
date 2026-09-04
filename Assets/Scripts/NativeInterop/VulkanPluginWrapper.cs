using System.Runtime.InteropServices;
using Unity.Collections;
using System;
using UnityEngine;
using UnityEngine.Rendering;

namespace Endfield.Rendering
{
    public static class VulkanPluginWrapper
    {
        private const string pluginName = "MiniEndfieldVulkanPlugin_v10";

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

        public static void InitializeWithDebug()
        {
            SetDebugLogCallback(_debugCallback);
        }

        [DllImport(pluginName)]
        public static extern IntPtr GetRenderEventFunc();

        [DllImport(pluginName)]
        public static extern void SetShaders(byte[] vertCode, int vertSize, byte[] fragCode, int fragSize);

        [DllImport(pluginName)]
        public static extern void SetResolution(int width, int height);

        [DllImport(pluginName)]
        public static extern void LoadMesh(uint meshId, IntPtr vertices, int vertexCount, IntPtr indices, int indexCount);

        [DllImport(pluginName)]
        public static extern void SubmitRenderBatch(IntPtr batchData, int instanceCount);
    }
}








