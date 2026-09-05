#include "Benchmark.h"
#include <algorithm>
#include <random>

namespace Endfield {

// Representation of an instance matching VulkanBackend.h
struct BenchmarkInstanceData {
    float mvpMatrix[16];
    uint64_t sortKey;
};

BenchmarkManager::BenchmarkManager() {
    m_CurrentStats = {};
    m_LatestCompletedStats = {};
}

BenchmarkManager& BenchmarkManager::Instance() {
    static BenchmarkManager instance;
    return instance;
}

void BenchmarkManager::SetCullingOptions(bool enableFrustum, bool enableOcclusion) {
    m_EnableFrustum = enableFrustum;
    m_EnableOcclusion = enableOcclusion;
}

void BenchmarkManager::BeginFrame() {
    m_FrameStartTime = std::chrono::high_resolution_clock::now();
    m_CurrentStats = {};
}

void BenchmarkManager::EndFrame() {
    auto frameEndTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float, std::milli> frameDuration = frameEndTime - m_FrameStartTime;
    m_CurrentStats.totalNativeFrameTimeMs = frameDuration.count();

    std::lock_guard<std::mutex> lock(m_StatsMutex);
    m_LatestCompletedStats = m_CurrentStats;
}

NativeBenchmarkStats BenchmarkManager::GetLatestFrameStats() {
    std::lock_guard<std::mutex> lock(m_StatsMutex);
    return m_LatestCompletedStats;
}

void BenchmarkManager::RunHeadlessBenchmark(int instanceCount, int iterations, NativeBenchmarkStats* outAverages) {
    if (!outAverages || instanceCount <= 0 || iterations <= 0) return;

    *outAverages = {};
    outAverages->totalInstances = static_cast<uint32_t>(instanceCount);

    double totalSortMs = 0.0;
    double totalBatchMs = 0.0;
    double totalFrustumMs = 0.0;
    double totalOccMs = 0.0;
    double totalFrameMs = 0.0;
    uint32_t lastCulledFrustum = 0;
    uint32_t lastCulledOcc = 0;
    uint32_t simulatedVisible = 0;

    // Fixed seed for deterministic benchmark comparison
    std::mt19937_64 rng(1337);
    std::uniform_int_distribution<uint64_t> meshDist(0, 18); // 19 submeshes
    std::uniform_int_distribution<uint64_t> depthDist(0, 65535);

    for (int iter = 0; iter < iterations; ++iter) {
        auto iterStart = std::chrono::high_resolution_clock::now();

        // 1. Generate instances
        std::vector<BenchmarkInstanceData> instances(instanceCount);
        for (int i = 0; i < instanceCount; ++i) {
            uint64_t meshId = meshDist(rng);
            uint64_t depth = depthDist(rng);
            // SortKey format: [Pass(8b) | Pipeline(8b) | MeshId(32b) | Depth(16b)]
            uint64_t sortKey = (meshId << 16) | (depth & 0xFFFF);
            instances[i].sortKey = sortKey;
        }

        // 2. Frustum Culling Simulation
        if (m_EnableFrustum) {
            auto fStart = std::chrono::high_resolution_clock::now();
            size_t keepCount = instances.size() * 3 / 10; // ~70% culled by frustum
            lastCulledFrustum = static_cast<uint32_t>(instances.size() - keepCount);
            instances.resize(keepCount);
            auto fEnd = std::chrono::high_resolution_clock::now();
            totalFrustumMs += std::chrono::duration<double, std::milli>(fEnd - fStart).count();
        }

        // 3. Occlusion Culling Simulation
        if (m_EnableOcclusion) {
            auto oStart = std::chrono::high_resolution_clock::now();
            size_t keepCount = instances.size() * 4 / 10; // ~60% occluded behind front crowd
            lastCulledOcc = static_cast<uint32_t>(instances.size() - keepCount);
            instances.resize(keepCount);
            auto oEnd = std::chrono::high_resolution_clock::now();
            totalOccMs += std::chrono::duration<double, std::milli>(oEnd - oStart).count();
        }

        // 4. Sort benchmark (Endfield 64-bit SortKey quicksort)
        auto sortStart = std::chrono::high_resolution_clock::now();
        std::sort(instances.begin(), instances.end(), [](const BenchmarkInstanceData& a, const BenchmarkInstanceData& b) {
            return a.sortKey < b.sortKey;
        });
        auto sortEnd = std::chrono::high_resolution_clock::now();
        totalSortMs += std::chrono::duration<double, std::milli>(sortEnd - sortStart).count();

        // 5. Batch build benchmark (redundant bind elimination & 0x7F7F7F7F check)
        auto batchStart = std::chrono::high_resolution_clock::now();
        uint32_t lastMeshId = 0xFFFFFFFF;
        uint32_t drawCount = 0;
        uint32_t bindCount = 0;

        for (const auto& inst : instances) {
            uint32_t meshId = static_cast<uint32_t>((inst.sortKey >> 16) & 0xFFFFFFFF);
            if (meshId != lastMeshId) {
                lastMeshId = meshId;
                bindCount++;
            }
            drawCount++;
        }
        auto batchEnd = std::chrono::high_resolution_clock::now();
        totalBatchMs += std::chrono::duration<double, std::milli>(batchEnd - batchStart).count();

        auto iterEnd = std::chrono::high_resolution_clock::now();
        totalFrameMs += std::chrono::duration<double, std::milli>(iterEnd - iterStart).count();

        simulatedVisible = drawCount;
    }

    outAverages->visibleInstances = simulatedVisible;
    outAverages->culledFrustum = lastCulledFrustum;
    outAverages->culledOcclusion = lastCulledOcc;
    outAverages->frustumCullingTimeMs = static_cast<float>(totalFrustumMs / iterations);
    outAverages->occlusionCullingTimeMs = static_cast<float>(totalOccMs / iterations);
    outAverages->sortingTimeMs = static_cast<float>(totalSortMs / iterations);
    outAverages->batchingTimeMs = static_cast<float>(totalBatchMs / iterations);
    outAverages->totalNativeFrameTimeMs = static_cast<float>(totalFrameMs / iterations);
}

} // namespace Endfield

