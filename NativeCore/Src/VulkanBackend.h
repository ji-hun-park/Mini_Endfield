#pragma once

#include <vector>
#include <vulkan/vulkan.h>

class VulkanBackend
{
public:
    VulkanBackend();
    ~VulkanBackend();

    void Initialize();
    void Shutdown();

    void SetupRenderGraph();
    
    void BeginFrame();
    void SubmitBatch(const void* batchData, int instanceCount);
    void EndFrame();

private:
    // Core Vulkan Handles
    VkInstance m_Instance = VK_NULL_HANDLE;
    VkDevice m_Device = VK_NULL_HANDLE;
    VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
    VkQueue m_GraphicsQueue = VK_NULL_HANDLE;
    VkCommandPool m_CommandPool = VK_NULL_HANDLE;
    VkCommandBuffer m_CommandBuffer = VK_NULL_HANDLE;
    
    // Manage descriptor sets tracking (Endfield redundant binding optimization)
    // 0x7F7F7F7F placeholder for redundant bindings
    uint32_t m_LastBoundMaterialSet = 0xFFFFFFFF;
    
    // For demonstration, a struct mimicking the submitted C# data
    struct InstanceData {
        float worldMatrix[16];
        uint64_t sortKey;
    };
};

