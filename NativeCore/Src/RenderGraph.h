#pragma once

#include <vector>
#include <string>

// Simulates the Render Graph concept mentioned in Endfield.md
// Merges barriers at compile/setup time based on Resource Access Tags

enum class AccessTag {
    ReadOnlyOptimal,
    WriteOptimal,
    Present,
    // etc...
};

struct ResourceNode {
    std::string name;
    AccessTag currentTag;
    bool isPersistent;
};

class RenderGraph {
public:
    void AddResource(const std::string& name, bool isPersistent) {
        // ...
    }

    void DeclarePassAccess(const std::string& passName, const std::string& resourceName, AccessTag tag) {
        // Collect access masks automatically.
    }

    void CompileGraph() {
        // Merge hazards within the same flush into a single pipeline barrier.
    }
};

