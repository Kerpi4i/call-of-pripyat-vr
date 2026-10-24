#pragma once
#include <cstdint>
namespace copvr_game {
struct ScopeAccuracyState {
    const void* weapon{}; uint32_t frame=uint32_t(-1),seenAt=0; bool visible=false;
};
inline ScopeAccuracyState& scopeAccuracyState() { static ScopeAccuracyState value; return value; }
inline void beginScopeAccuracyFrame(uint32_t frame) {
    auto& state=scopeAccuracyState();
    if(state.frame!=frame) { state.frame=frame; state.visible=false; state.weapon=nullptr; }
}
inline void markScopeAccuracy(const void* weapon,uint32_t now) {
    auto& state=scopeAccuracyState(); state.weapon=weapon; state.seenAt=now; state.visible=true;
}
inline bool usingScopeForAccuracy(const void* weapon,uint32_t now) {
    const auto& state=scopeAccuracyState();
    return state.visible&&state.weapon==weapon&&uint32_t(now-state.seenAt)<=150;
}
}
