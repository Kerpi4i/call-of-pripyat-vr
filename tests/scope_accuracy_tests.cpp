#include "game_vr_scope_accuracy.hpp"
#undef NDEBUG
#include <cassert>
int main() {
    using namespace copvr_game;
    int weapon,other;
    beginScopeAccuracyFrame(1);
    assert(!usingScopeForAccuracy(&weapon,100));
    markScopeAccuracy(&weapon,100);
    assert(usingScopeForAccuracy(&weapon,120));
    assert(!usingScopeForAccuracy(&other,120));
    beginScopeAccuracyFrame(1); // Other eye must not erase a valid first-eye observation.
    assert(usingScopeForAccuracy(&weapon,120));
    assert(!usingScopeForAccuracy(&weapon,251));
    beginScopeAccuracyFrame(2);
    assert(!usingScopeForAccuracy(&weapon,120));
    markScopeAccuracy(&weapon,0xfffffff0u);
    assert(usingScopeForAccuracy(&weapon,0x10u)); // Clock wrap.
}
