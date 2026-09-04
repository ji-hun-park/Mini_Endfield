#pragma once

#include <stdint.h>

#if defined(_MSC_VER)
    #define PLUGIN_API __declspec(dllexport)
#else
    #define PLUGIN_API
#endif

#include "IUnityInterface.h"
#include "IUnityGraphics.h"
#include "IUnityGraphicsVulkan.h"

#include "Benchmark.h"

extern "C" {
    
    // Unity Plugin Lifecycle
    PLUGIN_API void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API UnityPluginLoad(IUnityInterfaces* unityInterfaces);
    PLUGIN_API void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API UnityPluginUnload();

    // Render Event callback for CommandBuffer.IssuePluginEvent
    PLUGIN_API UnityRenderingEvent UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API GetRenderEventFunc();

    // Shader injection
    PLUGIN_API void SetShaders(const char* vertCode, int vertSize, const char* fragCode, int fragSize);
    PLUGIN_API void LoadMesh(uint32_t meshId, const float* vertices, int vertexCount, const uint32_t* indices, int indexCount);
    PLUGIN_API void SetViewProjectionMatrix(const float* vpMatrix);

    // Submit a chunk of sorted entities from Unity DOTS
    // In actual implementation, this takes a pointer to a NativeArray of structs
    PLUGIN_API void SubmitRenderBatch(const void* batchData, int instanceCount);

    typedef void(*DebugLogCallback)(const char*);
    PLUGIN_API void SetDebugLogCallback(DebugLogCallback callback);

    using NativeBenchmarkStats = Endfield::NativeBenchmarkStats;

    PLUGIN_API void GetLatestBenchmarkStats(NativeBenchmarkStats* outStats);
    PLUGIN_API void SetBenchmarkCullingOptions(bool enableFrustum, bool enableOcclusion);
    PLUGIN_API void RunNativeHeadlessBenchmark(int instanceCount, int iterations, NativeBenchmarkStats* outAverages);

}
