#pragma once
#include <algorithm>
#include "beam_math.hpp"
#include "vr_optics_limits.hpp"
#include <cmath>
#include <cstdint>
namespace copvr {
class BinocularZoom {
    float factor_=1.f; uint64_t last_=0; bool armed_=false;
public:
    float update(bool active,float stick,uint64_t timestamp) {
        if(!active||!std::isfinite(stick)) { last_=0; armed_=false; return factor_; }
        const float dt=last_&&timestamp>=last_?std::min(.05f,float(timestamp-last_)*.001f):0.f;
        last_=timestamp;
        if(std::abs(stick)<.25f) armed_=true;
        if(armed_&&std::abs(stick)>.3f) factor_=std::clamp(factor_+stick*dt*.4f,.5f,1.5f);
        return factor_;
    }
};
inline float binocularProximity(float eyeDistance) {
    if(!std::isfinite(eyeDistance)) return 0.f;
    const float t=std::clamp((.15f-eyeDistance)/.12f,0.f,1.f);
    return t*t*(3.f-2.f*t);
}
inline float binocularApertureRadius(float baseRadius,float eyeDistance) {
    return baseRadius+.01f*binocularProximity(eyeDistance);
}
inline Vec3 scopeTubeCoordinates(float pointX,float pointY,float eyeX,float eyeY,float eyeDistance,float radius) {
    if(!std::isfinite(eyeDistance)||!std::isfinite(eyeX)||!std::isfinite(eyeY)||eyeDistance<=.001f||radius<=0) return {8,8,0};
    constexpr float depth=.070f,openingScale=1.20f;
    const float travel=depth/eyeDistance;
    return {(pointX+(pointX-eyeX)*travel)/(radius*openingScale),
        (pointY+(pointY-eyeY)*travel)/(radius*openingScale),0};
}
inline Vec3 binocularSharedMask(float u,float v,float radius,float relief,float headX,float headY) {
    // Identical angular coordinates receive the same mask in both stereo images.
    const float halfY=std::clamp(.80f*radius/(2.f*.577350269f*std::max(.01f,relief)),.10f,.42f);
    const float shiftX=std::clamp(headX/.04f,-1.5f,1.5f);
    const float shiftY=std::clamp(headY/.04f,-1.5f,1.5f);
    return {(u-.5f)/(halfY*1.18f)+shiftX,(v-.5f)/halfY-shiftY,0};
}
inline float opticalPlaneSetback(float eyeRelief) {
    return std::isfinite(eyeRelief)?std::max(0.f,minimumOpticalPlane-eyeRelief):0.f;
}
inline bool binocularNear(Vec3 eye,Vec3 center,Vec3 forward,Vec3 look) {
    auto delta=center-eye; float distance=delta.length();
    if(!std::isfinite(distance)||distance<.001f||distance>.40f) return false;
    forward=unit(forward); look=unit(look);
    float axial=dot(delta,forward);
    return axial>.001f&&dot(forward,look)>.65f&&(delta-forward*axial).length()<.12f;
}
inline float binocularEyeLight(Vec3 eye,Vec3 center,Vec3 forward) {
    auto delta=eye-center; forward=unit(forward); float axial=-dot(delta,forward);
    if(!std::isfinite(axial)||axial<.001f||axial>.30f) return 0;
    float lateral=(delta+forward*axial).length();
    return std::clamp(1.f-lateral/.055f,0.f,1.f);
}
}
