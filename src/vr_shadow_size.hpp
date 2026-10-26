#pragma once
#include <cstdint>
namespace copvr {
// Owned by a light, used on the render thread. No cross-frame image reuse.
class FrameShadowSize {
    uint32_t frame_{},size_{};
    bool valid_{};
public:
    uint32_t select(uint32_t frame,uint32_t candidate) {
        if(!valid_||frame_!=frame){frame_=frame;size_=candidate;valid_=true;}
        return size_;
    }
};
}
