#pragma once
#include "interaction.hpp"
namespace copvr {
// A fresh trigger press or an offhand grab-and-pull primes the native grenade.
// Entering gameplay while buttons are held must never prime it immediately.
class GrenadePrimer {
    bool triggerReady_{},offReady_{},pulling_{},requested_{};
    Vec3 start_{};
public:
    bool update(float grip,float trigger,bool offAvailable,float offGrip,Vec3 relative) {
        if(requested_) return false;
        if(trigger<.25f) triggerReady_=true;
        if(grip>.7f&&triggerReady_&&trigger>.7f) { requested_=true; return true; }
        if(!offAvailable) { offReady_=pulling_=false; return false; }
        if(offGrip<.25f) { offReady_=true; pulling_=false; }
        if(grip>.7f&&offReady_&&offGrip>.7f) {
            offReady_=false;
            if(relative.length()<.14f) { start_=relative; pulling_=true; }
        }
        if(pulling_&&offGrip>.25f&&grip>.7f&&(relative-start_).length()>.09f) {
            requested_=true; return true;
        }
        return false;
    }
};
}
