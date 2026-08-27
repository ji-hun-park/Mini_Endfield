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
    
    for (auto& pair : m_Meshes) {
        if (pair.second.vertexBuffer) vkDestroyBuffer(m_Device, pair.second.vertexBuffer, nullptr);
        if (pair.second.vertexMemory) vkFreeMemory(m_Device, pair.second.vertexMemory, nullptr);
        if (pair.second.indexBuffer) vkDestroyBuffer(m_Device, pair.second.indexBuffer, nullptr);
        if (pair.second.indexMemory) vkFreeMemory(m_Device, pair.second.indexMemory, nullptr);
    }
    m_Meshes.clear();

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

    VkVertexInputBindingDescription bindingDescription{};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(Vertex);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    std::vector<VkVertexInputAttributeDescription> attributeDescriptions(2);
    // Position
    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(Vertex, pos);
    // UV
    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(Vertex, uv);

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

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

    VkPushConstantRange pushConstantRange{};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(float) * 16; // mat4

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

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
    VkPipelineDepthStencilStateCreateInfo depthStencil{}; depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO; depthStencil.depthTestEnable = VK_FALSE; depthStencil.depthWriteEnable = VK_FALSE; depthStencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL; pipelineInfo.pDepthStencilState = &depthStencil; pipelineInfo.pRasterizationState = &rasterizer;
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
    if (cmd == VK_NULL_HANDLE) { if (g_DebugCallback) g_DebugCallback("[VulkanBackend] cmd is NULL! Skipping."); return; }
    
    // Create pipeline on first render event if needed (requires Unity's active render pass)
    if (m_GraphicsPipeline == VK_NULL_HANDLE && recordingState.renderPass != VK_NULL_HANDLE) {
        m_RenderPass = recordingState.renderPass;
        CreateGraphicsPipeline();
    }

    if (m_GraphicsPipeline != VK_NULL_HANDLE) {
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_GraphicsPipeline);
        VkViewport viewport{}; viewport.width = (float)m_Width; viewport.height = (float)m_Height; viewport.minDepth = 0.0f; viewport.maxDepth = 1.0f; vkCmdSetViewport(cmd, 0, 1, &viewport);
        VkRect2D scissor{}; scissor.extent.width = m_Width; scissor.extent.height = m_Height; vkCmdSetScissor(cmd, 0, 1, &scissor);
        
        for (const auto& instance : m_SubmittedInstances) {
            uint32_t meshId = (instance.sortKey >> 16) & 0xFFFFFFFF; // assuming simple encoding
            auto it = m_Meshes.find(meshId);
            if (it == m_Meshes.end()) continue;

            const auto& mesh = it->second;
            VkBuffer vertexBuffers[] = { mesh.vertexBuffer };
            VkDeviceSize offsets[] = { 0 };
            vkCmdBindVertexBuffers(cmd, 0, 1, vertexBuffers, offsets);
            vkCmdBindIndexBuffer(cmd, mesh.indexBuffer, 0, VK_INDEX_TYPE_UINT32);

            // The InstanceData contains the fully computed MVP matrix from C#
            vkCmdPushConstants(cmd, m_PipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(float)*16, instance.mvpMatrix);
            vkCmdDrawIndexed(cmd, mesh.indexCount, 1, 0, 0, 0);
        }
    }
    m_SubmittedInstances.clear();
}

void VulkanBackend::SubmitBatch(const void* batchData, int instanceCount)
{
    const InstanceData* instances = reinterpret_cast<const InstanceData*>(batchData);
    m_SubmittedInstances.insert(m_SubmittedInstances.end(), instances, instances + instanceCount);
}

void VulkanBackend::SetupRenderGraph() {}
uint32_t VulkanBackend::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(m_PhysicalDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }
    return 0; // fallback, real app should throw
}

void VulkanBackend::LoadMesh(uint32_t meshId, const float* vertices, int vertexCount, const uint32_t* indices, int indexCount) {
    if (m_Device == VK_NULL_HANDLE || m_RenderPass == VK_NULL_HANDLE) return;
    

    MeshBuffers buffers{};
    buffers.indexCount = indexCount;

    // Vertex Buffer
    VkDeviceSize vertexBufferSize = sizeof(Vertex) * vertexCount;
    VkBufferCreateInfo vInfo{};
    vInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vInfo.size = vertexBufferSize;
    vInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    vInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    vkCreateBuffer(m_Device, &vInfo, nullptr, &buffers.vertexBuffer);

    VkMemoryRequirements vMemReqs;
    vkGetBufferMemoryRequirements(m_Device, buffers.vertexBuffer, &vMemReqs);

    VkMemoryAllocateInfo vAllocInfo{};
    vAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    vAllocInfo.allocationSize = vMemReqs.size;
    vAllocInfo.memoryTypeIndex = FindMemoryType(vMemReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    vkAllocateMemory(m_Device, &vAllocInfo, nullptr, &buffers.vertexMemory);
    vkBindBufferMemory(m_Device, buffers.vertexBuffer, buffers.vertexMemory, 0);

    void* vData;
    vkMapMemory(m_Device, buffers.vertexMemory, 0, vertexBufferSize, 0, &vData);
    memcpy(vData, vertices, (size_t)vertexBufferSize);
    vkUnmapMemory(m_Device, buffers.vertexMemory);

    // Index Buffer
    VkDeviceSize indexBufferSize = sizeof(uint32_t) * indexCount;
    VkBufferCreateInfo iInfo{};
    iInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    iInfo.size = indexBufferSize;
    iInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    iInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    vkCreateBuffer(m_Device, &iInfo, nullptr, &buffers.indexBuffer);

    VkMemoryRequirements iMemReqs;
    vkGetBufferMemoryRequirements(m_Device, buffers.indexBuffer, &iMemReqs);

    VkMemoryAllocateInfo iAllocInfo{};
    iAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    iAllocInfo.allocationSize = iMemReqs.size;
    iAllocInfo.memoryTypeIndex = FindMemoryType(iMemReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    vkAllocateMemory(m_Device, &iAllocInfo, nullptr, &buffers.indexMemory);
    vkBindBufferMemory(m_Device, buffers.indexBuffer, buffers.indexMemory, 0);

    void* iData;
    vkMapMemory(m_Device, buffers.indexMemory, 0, indexBufferSize, 0, &iData);
    memcpy(iData, indices, (size_t)indexBufferSize);
    vkUnmapMemory(m_Device, buffers.indexMemory);

    m_Meshes[meshId] = buffers;
    LogToUnity("[VulkanBackend] Mesh loaded and buffers created.");
}


void VulkanBackend::SetResolution(int width, int height) {
    m_Width = width;
    m_Height = height;
}














