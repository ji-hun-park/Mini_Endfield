#pragma once

#include <vector>

#if defined(_WIN32)
#define VK_USE_PLATFORM_WIN32_KHR
#endif
#include <vulkan/vulkan.h>
#include "IUnityGraphics.h"
#include "IUnityGraphicsVulkan.h"

class VulkanBackend
{
public:
    VulkanBackend();
    ~VulkanBackend();

    // Type for C# Debug.Log callback
    typedef void(*DebugLogFunc)(const char*);
    static void SetDebugCallback(DebugLogFunc callback);

    // Unity Plugin Lifecycle
    void OnPluginLoad(IUnityInterfaces* unityInterfaces);
    void OnPluginUnload();
    static void UNITY_INTERFACE_API OnGraphicsDeviceEvent(UnityGfxDeviceEventType eventType);
    
    // Unity Render Event
    void OnRenderEvent(int eventID);

    // Shader Injection
    void SetShaders(const char* vertCode, int vertSize, const char* fragCode, int fragSize);

    void SetupRenderGraph();
    void SubmitBatch(const void* batchData, int instanceCount);

private:
    void InitializeVulkan(IUnityInterfaces* unityInterfaces);
    
    // Unity interfaces
    IUnityInterfaces* m_UnityInterfaces = nullptr;
    IUnityGraphics* m_UnityGraphics = nullptr;
    IUnityGraphicsVulkan* m_UnityVulkan = nullptr;

    // Core Vulkan Handles (Provided by Unity)
    VkInstance m_Instance = VK_NULL_HANDLE;
    VkDevice m_Device = VK_NULL_HANDLE;
    VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
    VkQueue m_GraphicsQueue = VK_NULL_HANDLE;
    uint32_t m_GraphicsQueueFamilyIndex = 0;

    // Shader Storage
    std::vector<char> m_VertShaderCode;
    std::vector<char> m_FragShaderCode;

    // Render Pass & Pipeline
    VkRenderPass m_RenderPass = VK_NULL_HANDLE;
    VkPipeline m_GraphicsPipeline = VK_NULL_HANDLE;
    void CreateRenderPass(VkFormat format);
    void CreateGraphicsPipeline();
    VkShaderModule CreateShaderModule(const std::vector<char>& code);

    // We don't need our own CommandPool/Buffer or Sync objects anymore!
    // Unity handles the Swapchain and CommandBuffers.

    
    // Manage descriptor sets tracking (Endfield redundant binding optimization)
    // 0x7F7F7F7F placeholder for redundant bindings
    uint32_t m_LastBoundMaterialSet = 0xFFFFFFFF;
    
    // Placeholder Vulkan objects for demonstration of SubmitBatch
    VkPipelineLayout m_PipelineLayout = VK_NULL_HANDLE;
    VkDescriptorSet m_DescriptorSet0_Pass = VK_NULL_HANDLE;
    VkDescriptorSet m_DescriptorSet2_Object = VK_NULL_HANDLE;
    std::vector<VkDescriptorSet> m_MaterialSets;

    // For demonstration, a struct mimicking the submitted C# data
    struct InstanceData {
        float worldMatrix[16];
        uint64_t sortKey;
    };
};

