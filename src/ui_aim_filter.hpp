#pragma once
#include "adaptive_pose_filter.hpp"
#include <array>
namespace copvr {
// UI-only history. Never feed this back into gameplay hands, throws or melee.
class UiAimFilter {
    std::array<AdaptivePoseFilter,2> hands_;
    unsigned context_=0;
public:
    static AdaptivePoseSettings settings() {
        AdaptivePoseSettings s;
        s.positionCutoff=3.f; s.positionBeta=4.f;
        s.rotationCutoff=2.f; s.rotationBeta=1.f;
        s.maxPositionLag=.025f; s.maxRotationLag=.05235988f;
        return s;
    }
    void reset() { for(auto& hand:hands_) hand.reset(); context_=0; }
    void context(unsigned value) {
        if(value!=context_) { reset(); context_=value; }
    }
    Pose update(int side,Pose raw,uint64_t tick) {
        if(side<0||side>1||!context_) { raw.valid=false; return raw; }
        return hands_[side].update(raw,tick,settings());
    }
};
}
