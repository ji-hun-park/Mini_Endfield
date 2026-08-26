#pragma once

#include <stdint.h>

// 64-bit Sort Key structure mentioned in the Endfield documentation
union SortKey {
    uint64_t key;
    struct {
        uint64_t depth       : 16; // LSB: Depth/Distance
        uint64_t materialID  : 16; 
        uint64_t pipelineID  : 16; 
        uint64_t passID      : 16; // MSB: Screen/View Pass order
    } fields;
};

