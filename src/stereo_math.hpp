#pragma once
#include "interaction.hpp"
namespace copvr {
struct Clip { float x,y,z,w; };
struct Frustum { float left,right,up,down; };
inline Clip project(Vec3 world,Pose eye,Frustum f,float nearZ=0.05f,float farZ=100.f) {
    auto q=eye.orientation;
    Vec3 p=rotate({-q.x,-q.y,-q.z,q.w},world-eye.position);
    const float l=std::tan(f.left), r=std::tan(f.right), u=std::tan(f.up), d=std::tan(f.down);
    return {2*p.x/(r-l)+(r+l)*p.z/(r-l),2*p.y/(u-d)+(u+d)*p.z/(u-d),
        -farZ*p.z/(farZ-nearZ)-farZ*nearZ/(farZ-nearZ),-p.z};
}
}
