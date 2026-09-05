#pragma once

#include <vector>
#include <unordered_map>
#include <mutex>
#include <algorithm>

#ifndef NOMINMAX
#define NOMINMAX
#endif

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
    void SetResolution(int width, int height);
    int m_Width = 1920;
    int m_Height = 1080;

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
    void SetCullingOptions(bool enableFrustum, bool enableOcclusion);

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
    VkRenderPass m_NativeRenderPass = VK_NULL_HANDLE;
    VkPipeline m_GraphicsPipeline = VK_NULL_HANDLE;
    VkPipelineLayout m_PipelineLayout = VK_NULL_HANDLE;
    VkRenderPass CreateRenderPass(VkFormat colorFormat, VkFormat depthFormat);
    void CreateGraphicsPipeline();
    VkShaderModule CreateShaderModule(const std::vector<char>& code);

    // C++ Native Frustum & Occlusion Culling
    bool m_EnableNativeFrustumCulling = true;
    bool m_EnableNativeOcclusionCulling = true;
    static bool IsInsideFrustum(const float* mvp, float radius = 3.5f);
    void CullFrustumInstances();

    // Software Occlusion Culling (Pillar 2: 128x64 Low-Res Depth Buffer / 8-Tile Lock-Free)
    static constexpr int kOccWidth = 128;
    static constexpr int kOccHeight = 64;
    static constexpr int kOccTileCount = 8;
    alignas(64) float m_SoftwareDepthBuffer[kOccHeight][kOccWidth];

    void ClearSoftwareDepthBuffer();
    bool IsOccluded(const float* mvp, float radius) const;
    void RasterizeOccluder(const float* mvp, float radius);
    void CullOcclusionInstances();

    // Mesh Data
    struct Vertex {
        float pos[3];
        float uv[2];
    };

    struct MeshBuffers {
        VkBuffer vertexBuffer = VK_NULL_HANDLE;
        VkDeviceMemory vertexMemory = VK_NULL_HANDLE;
        VkBuffer indexBuffer = VK_NULL_HANDLE;
        VkDeviceMemory indexMemory = VK_NULL_HANDLE;
        uint32_t indexCount = 0;
    };

    std::unordered_map<uint32_t, MeshBuffers> m_Meshes;

public:
    void LoadMesh(uint32_t meshId, const float* vertices, int vertexCount, const uint32_t* indices, int indexCount);
private:
    uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);


    // We don't need our own CommandPool/Buffer or Sync objects anymore!
    // Unity handles the Swapchain and CommandBuffers.

    
    // Manage descriptor sets tracking (Endfield redundant binding optimization)
    // 0x7F7F7F7F placeholder for redundant bindings
    uint32_t m_LastBoundMaterialSet = 0xFFFFFFFF;
    
    // For demonstration, a struct mimicking the submitted C# data
    struct InstanceData {
        float mvpMatrix[16];
        uint64_t sortKey;
    };

    std::mutex m_BatchMutex;
    std::vector<InstanceData> m_PendingInstances;
    std::vector<InstanceData> m_RenderInstances;

    // GPU Instancing Dynamic Buffer
    VkBuffer m_InstanceBuffer = VK_NULL_HANDLE;
    VkDeviceMemory m_InstanceMemory = VK_NULL_HANDLE;
    size_t m_InstanceBufferCapacity = 0;
    void EnsureInstanceBuffer(size_t requiredCount);
};

