#include "PluginAPI.h"
#include "VulkanBackend.h"

static VulkanBackend* g_VulkanBackend = nullptr;

extern "C" {
    
    PLUGIN_API void SetDebugLogCallback(DebugLogCallback callback)
    {
        VulkanBackend::SetDebugCallback(callback);
    }

    static void UNITY_INTERFACE_API OnGraphicsDeviceEvent(UnityGfxDeviceEventType eventType)
    {
        if (g_VulkanBackend)
        {
            if (eventType == kUnityGfxDeviceEventInitialize) {
                g_VulkanBackend->OnPluginLoad(nullptr); // Handled differently now
            }
        }
    }

    // Called when the plugin is loaded by Unity
    PLUGIN_API void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API UnityPluginLoad(IUnityInterfaces* unityInterfaces)
    {
        if (!g_VulkanBackend)
        {
            g_VulkanBackend = new VulkanBackend();
        }
        g_VulkanBackend->OnPluginLoad(unityInterfaces);
        
        IUnityGraphics* graphics = unityInterfaces->Get<IUnityGraphics>();
        if (graphics) {
            graphics->RegisterDeviceEventCallback(OnGraphicsDeviceEvent);
    IUnityGraphicsVulkan* vulkan = unityInterfaces->Get<IUnityGraphicsVulkan>();
    if (vulkan) {
        UnityVulkanPluginEventConfig config{};
        config.renderPassPrecondition = kUnityVulkanRenderPass_EnsureInside;
        config.graphicsQueueAccess = kUnityVulkanGraphicsQueueAccess_DontCare;
        vulkan->ConfigureEvent(1, &config);
    }
            OnGraphicsDeviceEvent(kUnityGfxDeviceEventInitialize);
        }
    }

    // Called when the plugin is unloaded by Unity
    PLUGIN_API void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API UnityPluginUnload()
    {
        if (g_VulkanBackend)
        {
            g_VulkanBackend->OnPluginUnload();
            delete g_VulkanBackend;
            g_VulkanBackend = nullptr;
        }
    }

    // Called by Unity's CommandBuffer.IssuePluginEvent
    static void UNITY_INTERFACE_API OnRenderEvent(int eventID)
    {
        if (g_VulkanBackend)
        {
            g_VulkanBackend->OnRenderEvent(eventID);
        }
    }

    PLUGIN_API UnityRenderingEvent UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API GetRenderEventFunc()
    {
        return OnRenderEvent;
    }

    PLUGIN_API void SetShaders(const char* vertCode, int vertSize, const char* fragCode, int fragSize)
    {
        if (g_VulkanBackend)
        {
            g_VulkanBackend->SetShaders(vertCode, vertSize, fragCode, fragSize);
        }
    }

    PLUGIN_API void LoadMesh(uint32_t meshId, const float* vertices, int vertexCount, const uint32_t* indices, int indexCount)
    {
        if (g_VulkanBackend)
        {
            g_VulkanBackend->LoadMesh(meshId, vertices, vertexCount, indices, indexCount);
        }
    }

        PLUGIN_API void SubmitRenderBatch(const void* batchData, int instanceCount)
    {
        if (g_VulkanBackend)
        {
            g_VulkanBackend->SubmitBatch(batchData, instanceCount);
        }
    }

    PLUGIN_API void SetResolution(int width, int height)
    {
        if (g_VulkanBackend)
        {
            g_VulkanBackend->SetResolution(width, height);
        }
    }
}

