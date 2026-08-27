#pragma once

#include <stdint.h>

#if defined(_MSC_VER)
    #define PLUGIN_API __declspec(dllexport)
#else
    #define PLUGIN_API __attribute__((visibility("default")))
#endif

extern "C" {
    
    // Type for C# Debug.Log callback
    typedef void(*DebugLogCallback)(const char*);
    PLUGIN_API void SetDebugLogCallback(DebugLogCallback callback);

    // Initialize Vulkan backend (called by Unity)
    PLUGIN_API void InitializeVulkanBackend(void* windowHandle);

    // Shutdown Vulkan backend
    PLUGIN_API void ShutdownVulkanBackend();

    // Setup Render Graph (called at load time or when graph changes)
    PLUGIN_API void SetupRenderGraph();

    // Called every frame to begin recording commands
    PLUGIN_API void BeginFrame();

    // Submit a chunk of sorted entities from Unity DOTS
    // In actual implementation, this takes a pointer to a NativeArray of structs
    PLUGIN_API void SubmitRenderBatch(const void* batchData, int instanceCount);

    // End frame, submit command buffer to Vulkan queue, and present
    PLUGIN_API void EndFrame();

}
