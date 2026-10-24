#pragma once
#include <algorithm>
#include <cmath>
namespace copvr {
inline constexpr float chestHeight=-.29f,chestForward=0.f,chestHalfX=.22f,chestHalfY=.11f,chestHalfZ=.09f;
inline constexpr float backToolsHeight=-.61f,backToolsDepth=-.34f;
inline constexpr float backToolsHalfX=.21f,backToolsHalfY=.14f,backToolsHalfZ=.16f;
inline bool chestVolume(float x,float y,float z) {
    if(!std::isfinite(x)||!std::isfinite(y)||!std::isfinite(z)) return false;
    y-=chestHeight; z-=chestForward;
    return std::abs(x)<chestHalfX&&std::abs(y)<chestHalfY&&std::abs(z)<chestHalfZ;
}
inline bool backToolsVolume(float x,float y,float z) {
    return std::isfinite(x)&&std::isfinite(y)&&std::isfinite(z)&&
        std::abs(x)<backToolsHalfX&&std::abs(y-backToolsHeight)<backToolsHalfY&&std::abs(z-backToolsDepth)<backToolsHalfZ;
}
inline constexpr float quickHeight=-.535f,reloadRadius=.065f;
inline int quickZone(float x,float y,float z,int hand) {
    if(hand<0||hand>1||!std::isfinite(x)||!std::isfinite(y)||!std::isfinite(z)||
        std::abs(x)>=chestHalfX||std::abs(y-quickHeight)>=chestHalfY||std::abs(z-chestForward)>=chestHalfZ) return -1;
    return hand*2+(x<0?0:1);
}
inline constexpr float templeSide=.14f,templeForward=.05f,templeHalfX=.05f,templeHalfY=.17f,templeHalfZ=.11f;
inline bool templeVolume(float x,float y,float z) {
    return std::isfinite(x)&&std::isfinite(y)&&std::isfinite(z)&&
        std::abs(std::abs(x)-templeSide)<templeHalfX&&std::abs(y)<templeHalfY&&std::abs(z-templeForward)<templeHalfZ;
}
inline constexpr float weaponZoneHeightOffset=-.10f,weaponZoneForwardOffset=-.05f;
inline bool headWeaponVolume(float x,float y,float z,bool returning) {
    y-=weaponZoneHeightOffset; z-=weaponZoneForwardOffset;
    return std::isfinite(x)&&std::isfinite(y)&&std::isfinite(z)&&
        std::abs(x)<.46f&&y>=0.f&&y<.40f&&z>-.50f&&z<(returning?.26f:.02f);
}
inline float postureScale(float eyeDelta,float nativeEyeHeight) {
    if(!std::isfinite(eyeDelta)||!std::isfinite(nativeEyeHeight)||nativeEyeHeight<.5f) return 1.f;
    return std::clamp((nativeEyeHeight+eyeDelta)/nativeEyeHeight,.30f,1.f);
}
// Store heading in tracking space so engine snap turns apply immediately once.
struct TorsoHeading {
    float yaw=0; bool valid=false;
    float align(float target,float dt) {
        if(!valid) { yaw=target; valid=true; return yaw; }
        const float pi=3.141592654f;
        yaw=std::remainder(yaw+std::remainder(target-yaw,2*pi)*(1-std::exp(-5*std::clamp(dt,0.f,.05f))),2*pi);
        return yaw;
    }
    float alignWithinView(float target,float headYaw,float dt) {
        const float pi=3.141592654f,sector=pi/6;
        float difference=std::remainder(target-headYaw,2*pi);
        // Travel direction is an axis: walking backwards must not turn the torso around.
        if(difference>pi/2) difference-=pi;
        if(difference<-pi/2) difference+=pi;
        align(headYaw+std::clamp(difference,-sector,sector),dt);
        yaw=std::remainder(headYaw+std::clamp(std::remainder(yaw-headYaw,2*pi),-sector,sector),2*pi);
        return yaw;
    }
    float update(float headYaw,float dt,bool reset=false) {
        if(!valid||reset) { yaw=headYaw; valid=true; }
        const float pi=3.141592654f;
        float difference=std::remainder(headYaw-yaw,2*pi);
        const float deadzone=pi/6;
        if(std::abs(difference)>deadzone) {
            const float excess=difference-std::copysign(deadzone,difference);
            yaw+=excess*(1-std::exp(-6*std::clamp(dt,0.f,.05f)));
            yaw=std::remainder(yaw,2*pi);
        }
        return yaw;
    }
};
}
