#pragma once
#include <cstdint>
namespace copvr {
struct RainStep {
    std::uint32_t frame=0;
    bool initialized=false;
    bool advance(std::uint32_t current) {
        if(initialized&&frame==current) return false;
        initialized=true; frame=current; return true;
    }
};
}
