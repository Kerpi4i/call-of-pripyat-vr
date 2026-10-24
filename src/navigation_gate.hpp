#pragma once
#include "vr_bridge_api.h"
#include <cmath>
namespace copvr {
struct NavigationEdges { uint32_t pressed{},released{},held{}; int snap{}; float x{},y{}; };
class NavigationGate {
    bool armed_{},turnArmed_{}; uint32_t held_{};
public:
    NavigationEdges update(bool allowed,const CopVrNavigation& n) {
        NavigationEdges out;
        const bool finite=std::isfinite(n.moveX)&&std::isfinite(n.moveY)&&std::isfinite(n.turnX);
        if(!allowed||!finite) { out.released=held_; held_=0; armed_=turnArmed_=false; return out; }
        if(!armed_&&n.buttons==0&&std::abs(n.moveX)<.2f&&std::abs(n.moveY)<.2f&&std::abs(n.turnX)<.2f) armed_=true;
        if(!armed_) return out;
        out.pressed=n.buttons&~held_; out.released=held_&~n.buttons; held_=out.held=n.buttons;
        if(std::abs(n.turnX)<.25f) turnArmed_=true;
        if(turnArmed_&&std::abs(n.turnX)>.7f) { out.snap=n.turnX>0?1:-1; turnArmed_=false; }
        out.x=std::abs(n.moveX)>.3f?n.moveX:0; out.y=std::abs(n.moveY)>.3f?n.moveY:0;
        return out;
    }
};
class ButtonEdges {
    uint32_t held_{},armed_{};
public:
    uint32_t update(bool allowed,uint32_t buttons) {
        if(!allowed) { held_=armed_=0; return 0; }
        armed_|=~buttons;
        uint32_t pressed=buttons&~held_&armed_; held_=buttons; return pressed;
    }
};
}
