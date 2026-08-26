#include "PluginAPI.h"
#include "VulkanBackend.h"

static VulkanBackend* g_VulkanBackend = nullptr;

extern "C" {

    PLUGIN_API void InitializeVulkanBackend()
    {
        if (!g_VulkanBackend)
        {
            g_VulkanBackend = new VulkanBackend();
            g_VulkanBackend->Initialize();
        }
    }

    PLUGIN_API void ShutdownVulkanBackend()
    {
        if (g_VulkanBackend)
        {
            g_VulkanBackend->Shutdown();
            delete g_VulkanBackend;
            g_VulkanBackend = nullptr;
        }
    }

    PLUGIN_API void SetupRenderGraph()
    {
        if (g_VulkanBackend)
        {
            g_VulkanBackend->SetupRenderGraph();
        }
    }

    PLUGIN_API void BeginFrame()
    {
        if (g_VulkanBackend)
        {
            g_VulkanBackend->BeginFrame();
        }
    }

    PLUGIN_API void SubmitRenderBatch(const void* batchData, int instanceCount)
    {
        if (g_VulkanBackend)
        {
            g_VulkanBackend->SubmitBatch(batchData, instanceCount);
        }
    }

    PLUGIN_API void EndFrame()
    {
        if (g_VulkanBackend)
        {
            g_VulkanBackend->EndFrame();
        }
    }

}

