#pragma once

#include <vector>

#if defined(_WIN32)
#define VK_USE_PLATFORM_WIN32_KHR
#endif
#include <vulkan/vulkan.h>

class VulkanBackend
{
public:
    VulkanBackend();
    ~VulkanBackend();

    // Type for C# Debug.Log callback
    typedef void(*DebugLogFunc)(const char*);
    static void SetDebugCallback(DebugLogFunc callback);

    void Initialize(void* windowHandle = nullptr);
    void Shutdown();

    void SetupRenderGraph();
    
    void BeginFrame();
    void SubmitBatch(const void* batchData, int instanceCount);
    void EndFrame();

private:
    // Initialization Helpers
    void CreateInstance();
    void SetupDebugMessenger();
    void CreateSurface(void* windowHandle);
    void SelectPhysicalDevice();
    void CreateLogicalDevice();
    void CreateCommandObjects();
    void CreateSwapchain();

    // Core Vulkan Handles
    VkInstance m_Instance = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT m_DebugMessenger = VK_NULL_HANDLE;
    VkSurfaceKHR m_Surface = VK_NULL_HANDLE;
    VkDevice m_Device = VK_NULL_HANDLE;
    VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
    
    // Swapchain
    VkSwapchainKHR m_Swapchain = VK_NULL_HANDLE;
    VkFormat m_SwapchainImageFormat;
    VkExtent2D m_SwapchainExtent;
    std::vector<VkImage> m_SwapchainImages;
    std::vector<VkImageView> m_SwapchainImageViews;

    // Queues
    VkQueue m_GraphicsQueue = VK_NULL_HANDLE;
    VkQueue m_PresentQueue = VK_NULL_HANDLE;
    uint32_t m_GraphicsQueueFamilyIndex = 0;
    uint32_t m_PresentQueueFamilyIndex = 0;

    VkCommandPool m_CommandPool = VK_NULL_HANDLE;
    VkCommandBuffer m_CommandBuffer = VK_NULL_HANDLE;
    
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

