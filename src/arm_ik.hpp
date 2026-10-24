#pragma once
#include "beam_math.hpp"
#include <algorithm>
namespace copvr {
struct ArmSolution { bool valid{}; Vec3 shoulder,elbow,wrist; };
inline ArmSolution solveArm(Vec3 shoulder,Vec3 wrist,Vec3 hint,float upper,float lower) {
    if(!std::isfinite(upper)||!std::isfinite(lower)||upper<.01f||lower<.01f) return {};
    Vec3 delta=wrist-shoulder; float distance=delta.length();
    if(!std::isfinite(distance)) return {};
    Vec3 direction=distance>.0001f?delta*(1.f/distance):Vec3{0,0,1};
    float reach=std::clamp(distance,std::abs(upper-lower)+.001f,upper+lower-.001f);
    // Keep the wrist on the tracked controller; allow shoulder translation
    // beyond the mesh's reach rather than stretching the sleeve bones.
    shoulder=wrist-direction*reach;
    Vec3 bend=hint-shoulder; bend=unit(bend-direction*dot(bend,direction));
    if(bend.length()<.5f) bend=unit(cross(direction,std::abs(direction.y)<.9f?Vec3{0,1,0}:Vec3{1,0,0}));
    float along=(upper*upper-lower*lower+reach*reach)/(2*reach);
    float height=std::sqrt(std::max(0.f,upper*upper-along*along));
    return {true,shoulder,shoulder+direction*along+bend*height,wrist};
}
inline Quaternion rotationBetween(Vec3 from,Vec3 to) {
    from=unit(from); to=unit(to); float cosine=std::clamp(dot(from,to),-1.f,1.f);
    if(cosine<-.9999f) {
        auto axis=unit(cross(from,std::abs(from.y)<.9f?Vec3{0,1,0}:Vec3{1,0,0}));
        return {axis.x,axis.y,axis.z,0};
    }
    auto axis=cross(from,to); float scale=std::sqrt(2*(1+cosine));
    return {axis.x/scale,axis.y/scale,axis.z/scale,scale*.5f};
}
}
