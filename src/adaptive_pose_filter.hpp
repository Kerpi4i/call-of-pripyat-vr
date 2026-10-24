#pragma once
#include "beam_math.hpp"
#include <algorithm>
namespace copvr {
inline float poseQDot(Quaternion a,Quaternion b) { return a.x*b.x+a.y*b.y+a.z*b.z+a.w*b.w; }
inline Quaternion poseQUnit(Quaternion q) {
    const float n=std::sqrt(poseQDot(q,q)); return n>1e-6f?Quaternion{q.x/n,q.y/n,q.z/n,q.w/n}:Quaternion{};
}
inline float poseQAngle(Quaternion a,Quaternion b) { return 2.f*std::acos(std::clamp(std::abs(poseQDot(a,b)),0.f,1.f)); }
inline Quaternion poseQSlerp(Quaternion a,Quaternion b,float t) {
    float d=poseQDot(a,b); if(d<0) { b={-b.x,-b.y,-b.z,-b.w}; d=-d; }
    float x=1-t,y=t;
    if(d<.9995f) { const float theta=std::acos(std::clamp(d,0.f,1.f)),s=std::sin(theta); x=std::sin((1-t)*theta)/s; y=std::sin(t*theta)/s; }
    return poseQUnit({a.x*x+b.x*y,a.y*x+b.y*y,a.z*x+b.z*y,a.w*x+b.w*y});
}
struct AdaptivePoseSettings {
    float positionCutoff=12.f,positionBeta=20.f,rotationCutoff=10.f,rotationBeta=8.f;
    float maxPositionLag=.015f,maxRotationLag=.04363323f;
};
class AdaptivePoseFilter {
    bool valid_=false; uint64_t tick_=0; Pose raw_{},filtered_{}; Vec3 derivative_{}; float spin_=0;
    static float alpha(float cutoff,float dt) { return 1.f/(1.f+1.f/(6.283185307f*std::max(.01f,cutoff)*dt)); }
public:
    void reset() { valid_=false; derivative_={}; spin_=0; }
    Pose update(Pose raw,uint64_t tick,const AdaptivePoseSettings& settings={}) {
        if(!raw.valid||!std::isfinite(raw.position.length())||!std::isfinite(poseQDot(raw.orientation,raw.orientation))) { reset(); raw.valid=false; return raw; }
        raw.orientation=poseQUnit(raw.orientation);
        if(valid_&&tick==tick_) return filtered_;
        const float dt=valid_&&tick>tick_?float(tick-tick_)*.001f:0;
        if(!valid_||dt<=0||dt>.1f||(raw.position-raw_.position).length()>.35f||poseQAngle(raw.orientation,raw_.orientation)>1.5708f) {
            reset(); valid_=true; tick_=tick; raw_=filtered_=raw; return raw;
        }
        const Vec3 velocity=(raw.position-raw_.position)*(1.f/dt);
        const float angular=poseQAngle(raw.orientation,raw_.orientation)/dt;
        const float derivativeAlpha=alpha(1.f,dt);
        derivative_=derivative_+(velocity-derivative_)*derivativeAlpha;
        spin_+=(angular-spin_)*derivativeAlpha;
        const float p=alpha(settings.positionCutoff+settings.positionBeta*std::max(derivative_.length(),velocity.length()*.25f),dt);
        const float r=alpha(settings.rotationCutoff+settings.rotationBeta*std::max(spin_,angular*.25f),dt);
        filtered_.position=filtered_.position+(raw.position-filtered_.position)*p;
        filtered_.orientation=poseQSlerp(filtered_.orientation,raw.orientation,r);
        Vec3 error=filtered_.position-raw.position; const float distance=error.length();
        if(distance>settings.maxPositionLag) filtered_.position=raw.position+error*(settings.maxPositionLag/distance);
        const float angle=poseQAngle(raw.orientation,filtered_.orientation);
        if(angle>settings.maxRotationLag) filtered_.orientation=poseQSlerp(raw.orientation,filtered_.orientation,settings.maxRotationLag/angle);
        raw_=raw; tick_=tick; return filtered_;
    }
};
}
