#pragma once
#include "beam_math.hpp"
namespace copvr {
inline float knifeDamageScale(float speed) {
    return std::isfinite(speed)&&speed>=.8f?std::clamp(speed/3.f,0.f,2.f):0.f;
}
class KnifeSwingGate {
    bool armed_=false; float rest_=0,elapsed_=1; Vec3 impact_{};
public:
    void reset() { *this=KnifeSwingGate{}; }
    bool update(Vec3 velocity,float dt) {
        const float speed=velocity.length();
        if(!std::isfinite(speed)||dt<=0||dt>.1f||speed>12.f) { reset(); return false; }
        elapsed_+=dt;
        if(speed<.35f) { rest_+=dt; if(rest_>=.08f&&elapsed_>=.18f) armed_=true; }
        else rest_=0;
        if(!armed_&&elapsed_>=.18f&&speed>=.8f&&impact_.length()>.1f&&dot(unit(velocity),unit(impact_))<-.4f) armed_=true;
        return armed_&&speed>=.8f&&elapsed_>=.18f;
    }
    void contact(Vec3 velocity) { armed_=false; elapsed_=0; rest_=0; impact_=velocity; }
};
}
