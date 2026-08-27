#include "VulkanBackend.h"
#include <iostream>
#include <string>

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

VulkanBackend::VulkanBackend() {}
VulkanBackend::~VulkanBackend() {}

void VulkanBackend::OnPluginLoad(IUnityInterfaces* unityInterfaces)
{
    if (!unityInterfaces) return;
    m_UnityInterfaces = unityInterfaces;
    m_UnityGraphics = m_UnityInterfaces->Get<IUnityGraphics>();
    
    if (m_UnityGraphics && m_UnityGraphics->GetRenderer() == kUnityGfxRendererVulkan) {
        InitializeVulkan(m_UnityInterfaces);
    }
}

void VulkanBackend::InitializeVulkan(IUnityInterfaces* unityInterfaces)
{
    m_UnityVulkan = unityInterfaces->Get<IUnityGraphicsVulkan>();
    if (!m_UnityVulkan) return;

    UnityVulkanInstance vulkanInstance = m_UnityVulkan->Instance();
    m_Instance = vulkanInstance.instance;
    m_PhysicalDevice = vulkanInstance.physicalDevice;
    m_Device = vulkanInstance.device;
    m_GraphicsQueue = vulkanInstance.graphicsQueue;
    m_GraphicsQueueFamilyIndex = vulkanInstance.queueFamilyIndex;

    LogToUnity("[VulkanBackend] Successfully hooked into Unity's Vulkan Device.");
}

void VulkanBackend::OnPluginUnload()
{
    if (m_Device) {
        vkDeviceWaitIdle(m_Device);
    }
    
    if (m_GraphicsPipeline && m_Device) {
        vkDestroyPipeline(m_Device, m_GraphicsPipeline, nullptr);
        m_GraphicsPipeline = VK_NULL_HANDLE;
    }
    if (m_PipelineLayout && m_Device) {
        vkDestroyPipelineLayout(m_Device, m_PipelineLayout, nullptr);
        m_PipelineLayout = VK_NULL_HANDLE;
    }
}

void VulkanBackend::SetShaders(const char* vertCode, int vertSize, const char* fragCode, int fragSize)
{
    m_VertShaderCode.assign(vertCode, vertCode + vertSize);
    m_FragShaderCode.assign(fragCode, fragCode + fragSize);
    LogToUnity("[VulkanBackend] Shaders received from C#.");
}

VkShaderModule VulkanBackend::CreateShaderModule(const std::vector<char>& code) {
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(m_Device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        LogToUnity("[VulkanBackend ERROR] Failed to create shader module!");
        return VK_NULL_HANDLE;
    }
    return shaderModule;
}

void VulkanBackend::CreateGraphicsPipeline()
{
    if (m_Device == VK_NULL_HANDLE || m_RenderPass == VK_NULL_HANDLE) return;
    if (m_VertShaderCode.empty() || m_FragShaderCode.empty()) return;

    VkShaderModule vertShaderModule = CreateShaderModule(m_VertShaderCode);
    VkShaderModule fragShaderModule = CreateShaderModule(m_FragShaderCode);

    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_NONE;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;

    std::vector<VkDynamicState> dynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

    if (vkCreatePipelineLayout(m_Device, &pipelineLayoutInfo, nullptr, &m_PipelineLayout) != VK_SUCCESS) {
        LogToUnity("[VulkanBackend ERROR] Failed to create pipeline layout!");
    }

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = m_PipelineLayout;
    pipelineInfo.renderPass = m_RenderPass;
    pipelineInfo.subpass = 0;

    if (vkCreateGraphicsPipelines(m_Device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_GraphicsPipeline) != VK_SUCCESS) {
        LogToUnity("[VulkanBackend ERROR] Failed to create graphics pipeline!");
    } else {
        LogToUnity("[VulkanBackend] Graphics Pipeline created via Unity RenderPass.");
    }

    vkDestroyShaderModule(m_Device, fragShaderModule, nullptr);
    vkDestroyShaderModule(m_Device, vertShaderModule, nullptr);
}

void VulkanBackend::OnRenderEvent(int eventID)
{
    if (!m_UnityVulkan) return;

    UnityVulkanRecordingState recordingState;
    // We request access to the queue if we needed it, but here we just want the command buffer state
    if (!m_UnityVulkan->CommandRecordingState(&recordingState, kUnityVulkanGraphicsQueueAccess_DontCare)) {
        return;
    }

    VkCommandBuffer cmd = recordingState.commandBuffer;
    
    // Create pipeline on first render event if needed (requires Unity's active render pass)
    if (m_GraphicsPipeline == VK_NULL_HANDLE && recordingState.renderPass != VK_NULL_HANDLE) {
        m_RenderPass = recordingState.renderPass;
        CreateGraphicsPipeline();
    }

    if (m_GraphicsPipeline != VK_NULL_HANDLE) {
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_GraphicsPipeline);
        
        // We assume Unity has set the dynamic viewport/scissor already, but just in case:
        // Wait, Unity sets them if kUnityVulkanRenderPass_EnsureInside is used.
        // We will just draw our hardcoded triangle here for demonstration
        vkCmdDraw(cmd, 3, 1, 0, 0);
    }
}

void VulkanBackend::SubmitBatch(const void* batchData, int instanceCount)
{
    // Will be integrated into OnRenderEvent in future steps
}

void VulkanBackend::SetupRenderGraph() {}
