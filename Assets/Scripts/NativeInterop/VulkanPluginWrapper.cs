using System.Runtime.InteropServices;
using Unity.Collections;
using System;

namespace Endfield.Rendering
{
    public static class VulkanPluginWrapper
    {
        private const string pluginName = "MiniEndfieldVulkanPlugin";

        [DllImport(pluginName)]
        public static extern void InitializeVulkanBackend();

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

