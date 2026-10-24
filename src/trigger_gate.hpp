#pragma once
#include <cmath>
namespace copvr {
// Edges: +1=start, -1=stop, 0=no change. Releasing is required after inhibition.
class TriggerGate {
    bool armed_{},pressed_{};
public:
    bool pressed() const { return pressed_; }
    int reset() { const int result=pressed_?-1:0; armed_=pressed_=false; return result; }
    int update(bool allowed,float trigger) {
        if(!allowed||!std::isfinite(trigger)) return reset();
        if(trigger<=.2f) armed_=true;
        const bool next=armed_&&(pressed_?trigger>.2f:trigger>=.75f);
        if(next==pressed_) return 0;
        pressed_=next; return next?1:-1;
    }
};
}
