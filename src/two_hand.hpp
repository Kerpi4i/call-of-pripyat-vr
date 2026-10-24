#pragma once
#include "beam_math.hpp"
namespace copvr {
class SupportGrip {
    bool active_{},armed_{};
public:
    void reset() { active_=armed_=false; }
    bool active() const { return active_; }
    bool update(bool tracked,Vec3 primary,Vec3 support,Vec3 aim,float squeeze,bool nearForegrip=true) {
        auto delta=support-primary; float distance=delta.length();
        if(!tracked||!std::isfinite(squeeze)||!std::isfinite(distance)) { reset(); return false; }
        if(squeeze<.25f) { active_=false; armed_=true; }
        if(active_ && (distance<.04f||distance>.85f)) { reset(); return false; }
        if(!active_&&armed_&&squeeze>=.7f) {
            float along=dot(delta,unit(aim));
            // The model-specific palm socket supplies the narrow acquisition
            // region. A second generic cone rejected short weapons entirely.
            if(nearForegrip&&along>=.03f&&distance<=.75f) active_=true;
            armed_=false; // a missed grab requires releasing before retrying
        }
        return active_;
    }
};
}
