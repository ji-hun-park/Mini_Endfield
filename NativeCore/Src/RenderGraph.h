#pragma once

#include <vector>
#include <string>
#include <unordered_map>
#include <iostream>
#include <stdexcept>

// Simulates the Render Graph concept mentioned in Endfield.md
// Merges barriers at compile/setup time based on Resource Access Tags

enum class AccessTag {
    None,
    ColorAttachmentWrite,
    DepthStencilWrite,
    DepthStencilRead,
    ShaderRead,
    TransferRead,
    TransferWrite,
    Present
};

struct ResourceAccess {
    std::string resourceName;
    AccessTag requestedTag;
};

struct ResourceState {
    std::string name;
    AccessTag currentTag;
    bool isPersistent;
};

struct BarrierHazard {
    std::string resourceName;
    AccessTag before;
    AccessTag after;
};

struct RenderPassNode {
    std::string passName;
    std::vector<ResourceAccess> accesses;
    
    // Barriers that need to be flushed BEFORE this pass executes.
    // By merging hazards, we issue just ONE pipeline barrier command containing all these transitions.
    std::vector<BarrierHazard> mergedBarriers; 
};

class RenderGraph {
private:
    std::unordered_map<std::string, ResourceState> resources;
    std::vector<RenderPassNode> passes;

public:
    void AddResource(const std::string& name, bool isPersistent, AccessTag initialTag = AccessTag::None) {
        if (resources.find(name) != resources.end()) {
            std::cerr << "Resource already exists: " << name << "\n";
            return;
        }
        resources[name] = {name, initialTag, isPersistent};
    }

    void AddPass(const std::string& passName) {
        passes.push_back({passName, {}, {}});
    }

    void DeclarePassAccess(const std::string& passName, const std::string& resourceName, AccessTag tag) {
        for (auto& pass : passes) {
            if (pass.passName == passName) {
                pass.accesses.push_back({resourceName, tag});
                return;
            }
        }
        std::cerr << "Pass not found: " << passName << "\n";
    }

    void CompileGraph() {
        std::cout << "[RenderGraph] Compiling graph and merging barriers...\n";

        for (auto& pass : passes) {
            for (const auto& access : pass.accesses) {
                auto it = resources.find(access.resourceName);
                if (it == resources.end()) {
                    std::cerr << "Resource used but not declared: " << access.resourceName << "\n";
                    continue;
                }

                ResourceState& res = it->second;

                // Check if a state transition is needed
                if (res.currentTag != access.requestedTag) {
                    
                    // Optimization: If both are read-only, skip transition
                    bool isOldRead = (res.currentTag == AccessTag::ShaderRead || res.currentTag == AccessTag::DepthStencilRead);
                    bool isNewRead = (access.requestedTag == AccessTag::ShaderRead || access.requestedTag == AccessTag::DepthStencilRead);
                    
                    if (isOldRead && isNewRead) {
                        continue; // No hazard, barrier can be skipped
                    }

                    // Hazard detected: Create a barrier and add to the pass's merged barrier list
                    pass.mergedBarriers.push_back({res.name, res.currentTag, access.requestedTag});
                    
                    // Update resource tracking state
                    res.currentTag = access.requestedTag;
                }
            }

            // At this point, pass.mergedBarriers contains all hazards for this flush.
            // In a real Vulkan backend, we would convert pass.mergedBarriers into a single vkCmdPipelineBarrier call.
            if (!pass.mergedBarriers.empty()) {
                std::cout << "  -> Pass [" << pass.passName << "] requires a merged barrier for " 
                          << pass.mergedBarriers.size() << " resources.\n";
            }
        }
        
        std::cout << "[RenderGraph] Compilation successful.\n";
    }
    
    const std::vector<RenderPassNode>& GetPasses() const {
        return passes;
    }
};
