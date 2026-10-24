#pragma once
#include <cmath>
namespace copvr {
inline bool canCalibrateStanding(float eyeDelta){return std::isfinite(eyeDelta)&&eyeDelta>=-.18f;}
struct PhysicalPostureGate {
    bool crouched=false;
    bool update(bool tracked,float eyeDelta) {
        if(!tracked||!std::isfinite(eyeDelta)){crouched=false;return false;}
        if(eyeDelta<-.25f)crouched=true;else if(eyeDelta>-.18f)crouched=false;
        return crouched;
    }
};
}
