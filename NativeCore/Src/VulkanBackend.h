#pragma once

#include <vector>

// Forward declarations for Vulkan types, to avoid requiring vulkan.h in header if possible,
// but for simplicity we can include it if needed. 
// Assuming a simplified structure for demonstration.

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
    // This is where explicit Vulkan handles would reside.
    // VkInstance instance;
    // VkDevice device;
    // VkQueue graphicsQueue;
    // VkCommandBuffer currentCommandBuffer;
    
    // Manage descriptor sets:
    // Set 0: Per Pass
    // Set 1: Per Material
    // Set 2: Per Draw (Dynamic Offset)
};

