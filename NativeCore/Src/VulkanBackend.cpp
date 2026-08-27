#include "VulkanBackend.h"
#include "RenderGraph.h"
#include <iostream>
#include <string>
#include <vector>

static VulkanBackend::DebugLogFunc g_DebugCallback = nullptr;

void VulkanBackend::SetDebugCallback(DebugLogFunc callback) {
    g_DebugCallback = callback;
}

static void LogToUnity(const std::string& message) {
    if (g_DebugCallback) {
        g_DebugCallback(message.c_str());
    } else {
        std::cout << message << std::endl;
    }
}

static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) 
{
    std::string prefix = "[Vulkan] ";
    if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) prefix = "[Vulkan ERROR] ";
    else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) prefix = "[Vulkan WARNING] ";

    LogToUnity(prefix + pCallbackData->pMessage);
    return VK_FALSE;
}

VulkanBackend::VulkanBackend()
{
}

VulkanBackend::~VulkanBackend()
{
}

void VulkanBackend::Initialize()
{
    CreateInstance();
    SetupDebugMessenger();
    SelectPhysicalDevice();
    CreateLogicalDevice();
    CreateCommandObjects();

    LogToUnity("[VulkanBackend] Successfully Initialized Native Vulkan Backend.");
}

void VulkanBackend::CreateInstance()
{
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Mini Endfield Native Renderer";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "Endfield Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    // Enable Validation Layers
    const char* validationLayers[] = { "VK_LAYER_KHRONOS_validation" };
    createInfo.enabledLayerCount = 1;
    createInfo.ppEnabledLayerNames = validationLayers;

    // Enable Debug Utils Extension
    const char* extensions[] = { VK_EXT_DEBUG_UTILS_EXTENSION_NAME };
    createInfo.enabledExtensionCount = 1;
    createInfo.ppEnabledExtensionNames = extensions;

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    debugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    debugCreateInfo.pfnUserCallback = DebugCallback;
    
    createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;

    if (vkCreateInstance(&createInfo, nullptr, &m_Instance) != VK_SUCCESS) {
        LogToUnity("[VulkanBackend] Failed to create Vulkan instance!");
    }
}

void VulkanBackend::SetupDebugMessenger()
{
    if (m_Instance == VK_NULL_HANDLE) return;

    VkDebugUtilsMessengerCreateInfoEXT createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = DebugCallback;

    auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(m_Instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(m_Instance, &createInfo, nullptr, &m_DebugMessenger);
        LogToUnity("[VulkanBackend] Debug Messenger successfully created.");
    } else {
        LogToUnity("[VulkanBackend ERROR] Failed to load vkCreateDebugUtilsMessengerEXT function.");
    }
}

void VulkanBackend::SelectPhysicalDevice()
{
    if (m_Instance == VK_NULL_HANDLE) return;

    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(m_Instance, &deviceCount, nullptr);
    if (deviceCount == 0) {
        std::cerr << "[VulkanBackend] Failed to find GPUs with Vulkan support!\n";
        return;
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(m_Instance, &deviceCount, devices.data());
    m_PhysicalDevice = devices[0]; // Simply pick the first available device
}

void VulkanBackend::CreateLogicalDevice()
{
    if (m_PhysicalDevice == VK_NULL_HANDLE) return;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevice, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevice, &queueFamilyCount, queueFamilies.data());

    uint32_t graphicsQueueFamilyIndex = 0;
    for (uint32_t i = 0; i < queueFamilyCount; i++) {
        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            graphicsQueueFamilyIndex = i;
            break;
        }
    }

    float queuePriority = 1.0f;
    VkDeviceQueueCreateInfo queueCreateInfo{};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = graphicsQueueFamilyIndex;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &queuePriority;

    VkDeviceCreateInfo deviceCreateInfo{};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
    deviceCreateInfo.queueCreateInfoCount = 1;

    if (vkCreateDevice(m_PhysicalDevice, &deviceCreateInfo, nullptr, &m_Device) != VK_SUCCESS) {
        std::cerr << "[VulkanBackend] Failed to create logical device!\n";
        return;
    }
    vkGetDeviceQueue(m_Device, graphicsQueueFamilyIndex, 0, &m_GraphicsQueue);
}

void VulkanBackend::CreateCommandObjects()
{
    if (m_Device == VK_NULL_HANDLE || m_PhysicalDevice == VK_NULL_HANDLE) return;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevice, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevice, &queueFamilyCount, queueFamilies.data());

    uint32_t graphicsQueueFamilyIndex = 0;
    for (uint32_t i = 0; i < queueFamilyCount; i++) {
        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            graphicsQueueFamilyIndex = i;
            break;
        }
    }

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = graphicsQueueFamilyIndex;

    if (vkCreateCommandPool(m_Device, &poolInfo, nullptr, &m_CommandPool) != VK_SUCCESS) {
        std::cerr << "[VulkanBackend] Failed to create command pool!\n";
        return;
    }

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = m_CommandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    if (vkAllocateCommandBuffers(m_Device, &allocInfo, &m_CommandBuffer) != VK_SUCCESS) {
        std::cerr << "[VulkanBackend] Failed to allocate command buffers!\n";
    }
}

void VulkanBackend::Shutdown()
{
    // Wait for the logical device to finish operations before cleaning up
    if (m_Device) {
        vkDeviceWaitIdle(m_Device);
        
        if (m_CommandPool) {
            vkDestroyCommandPool(m_Device, m_CommandPool, nullptr);
            m_CommandPool = VK_NULL_HANDLE;
        }
        vkDestroyDevice(m_Device, nullptr);
        m_Device = VK_NULL_HANDLE;
    }

    if (m_Instance) {
        vkDestroyInstance(m_Instance, nullptr);
        m_Instance = VK_NULL_HANDLE;
    }

    std::cout << "[VulkanBackend] Shut down native Vulkan backend.\n";
}

void VulkanBackend::SetupRenderGraph()
{
    RenderGraph graph;

    // 1. Declare resources
    graph.AddResource("GBufferColor", true, AccessTag::None);
    graph.AddResource("GBufferDepth", true, AccessTag::None);
    graph.AddResource("ShadowMap", false, AccessTag::None);

    // 2. Declare passes and their resource accesses
    graph.AddPass("ShadowPass");
    graph.DeclarePassAccess("ShadowPass", "ShadowMap", AccessTag::DepthStencilWrite);

    graph.AddPass("OpaquePass");
    graph.DeclarePassAccess("OpaquePass", "GBufferColor", AccessTag::ColorAttachmentWrite);
    graph.DeclarePassAccess("OpaquePass", "GBufferDepth", AccessTag::DepthStencilWrite);
    graph.DeclarePassAccess("OpaquePass", "ShadowMap", AccessTag::ShaderRead); // Needs transition!

    graph.AddPass("LightingPass");
    graph.DeclarePassAccess("LightingPass", "GBufferColor", AccessTag::ShaderRead); // Needs transition!
    graph.DeclarePassAccess("LightingPass", "GBufferDepth", AccessTag::ShaderRead); // Needs transition!

    // 3. Compile the graph to merge barriers
    graph.CompileGraph();

    // The merged barriers would now be used to generate explicit vkCmdPipelineBarrier calls
    // exactly at the boundaries between passes.
    
    std::cout << "[VulkanBackend] Render Graph setup complete. Barriers merged at compile time.\n";
}

void VulkanBackend::BeginFrame()
{
    if (m_CommandBuffer == VK_NULL_HANDLE) return;

    // Reset command buffer before starting new frame commands
    vkResetCommandBuffer(m_CommandBuffer, 0);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    if (vkBeginCommandBuffer(m_CommandBuffer, &beginInfo) != VK_SUCCESS) {
        std::cerr << "[VulkanBackend] Failed to begin recording command buffer!\n";
    }
    
    // Reset our redundant binding tracker for the new frame
    m_LastBoundMaterialSet = 0xFFFFFFFF;
}

void VulkanBackend::SubmitBatch(const void* batchData, int instanceCount)
{
    if (instanceCount == 0 || !batchData || m_CommandBuffer == VK_NULL_HANDLE) return;

    const InstanceData* instances = static_cast<const InstanceData*>(batchData);

    // Endfield Architecture: Descriptor sets are separated by frequency of update
    // Set 0: Per Pass (Lighting, Camera, Shadows)
    // Set 1: Per Material (Textures, Constants)
    // Set 2: Per Draw (Object matrices via Dynamic Offset)

    // E.g., Bind Set 0 (Per Pass) once per batch/pass
    if (m_PipelineLayout != VK_NULL_HANDLE && m_DescriptorSet0_Pass != VK_NULL_HANDLE) {
        vkCmdBindDescriptorSets(m_CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, 
                                m_PipelineLayout, 0, 1, &m_DescriptorSet0_Pass, 0, nullptr);
    }

    for (int i = 0; i < instanceCount; ++i)
    {
        const InstanceData& data = instances[i];
        
        // Decode the 64-bit sort key to get material ID
        // Format [63:48 Pass] [47:32 Pipeline] [31:16 Material] [15:0 Depth]
        uint32_t materialID = (data.sortKey >> 16) & 0xFFFF;
        
        // Redundant binding optimization (Placeholder value 0x7F7F7F7F logic)
        // If the parallel worker encountered this same material earlier, we don't bind again.
        if (materialID != m_LastBoundMaterialSet && materialID != 0x7F7F7F7F)
        {
            // Bind Set 1 (Material)
            if (m_PipelineLayout != VK_NULL_HANDLE && materialID < m_MaterialSets.size()) {
                VkDescriptorSet materialSet = m_MaterialSets[materialID];
                vkCmdBindDescriptorSets(m_CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, 
                                        m_PipelineLayout, 1, 1, &materialSet, 0, nullptr);
            }
            m_LastBoundMaterialSet = materialID;
        }

        // Set 2 (Object Data): Dynamic Offset based on instance index
        uint32_t dynamicOffset = i * sizeof(InstanceData);
        
        if (m_PipelineLayout != VK_NULL_HANDLE && m_DescriptorSet2_Object != VK_NULL_HANDLE) {
            vkCmdBindDescriptorSets(m_CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, 
                                    m_PipelineLayout, 2, 1, &m_DescriptorSet2_Object, 1, &dynamicOffset);
        }

        // Execute Draw Call
        // Assuming indexCount is managed externally or stored in the material/mesh data.
        // For demonstration, we use a mock indexCount of 36 (e.g., a cube).
        uint32_t indexCount = 36;
        vkCmdDrawIndexed(m_CommandBuffer, indexCount, 1, 0, 0, 0);
    }
}

void VulkanBackend::EndFrame()
{
    if (m_CommandBuffer == VK_NULL_HANDLE) return;

    if (vkEndCommandBuffer(m_CommandBuffer) != VK_SUCCESS) {
        std::cerr << "[VulkanBackend] Failed to record command buffer!\n";
        return;
    }
    
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &m_CommandBuffer;
    
    if (vkQueueSubmit(m_GraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS) {
        std::cerr << "[VulkanBackend] Failed to submit draw command buffer!\n";
    }
    
    // Simplistic CPU wait to guarantee queue completion for this skeleton
    // In production, multi-buffering with VkFence would be used instead of stalling
    vkQueueWaitIdle(m_GraphicsQueue);
    
    // Note: vkQueuePresentKHR would go here if rendering directly to a screen swapchain.
    // Usually with Unity plugins, we render to a shared offscreen texture instead.
}
