#pragma once
#include "interaction.hpp"
namespace copvr {
inline float dot(Vec3 a,Vec3 b) { return a.x*b.x+a.y*b.y+a.z*b.z; }
inline Vec3 cross(Vec3 a,Vec3 b) { return {a.y*b.z-a.z*b.y,a.z*b.x-a.x*b.z,a.x*b.y-a.y*b.x}; }
inline Vec3 unit(Vec3 a) { float n=a.length(); return n>.00001f?a*(1.f/n):Vec3{}; }
inline Quaternion basisQuaternion(Vec3 x,Vec3 y,Vec3 z) {
    Quaternion q; float trace=x.x+y.y+z.z;
    if(trace>0) { float s=std::sqrt(trace+1)*2; q={(y.z-z.y)/s,(z.x-x.z)/s,(x.y-y.x)/s,.25f*s}; }
    else if(x.x>y.y && x.x>z.z) { float s=std::sqrt(1+x.x-y.y-z.z)*2; q={.25f*s,(y.x+x.y)/s,(z.x+x.z)/s,(y.z-z.y)/s}; }
    else if(y.y>z.z) { float s=std::sqrt(1+y.y-x.x-z.z)*2; q={(y.x+x.y)/s,.25f*s,(z.y+y.z)/s,(z.x-x.z)/s}; }
    else { float s=std::sqrt(1+z.z-x.x-y.y)*2; q={(z.x+x.z)/s,(z.y+y.z)/s,.25f*s,(x.y-y.x)/s}; }
    return q;
}
// Quad local Y follows the segment; +Z faces the viewer.
inline Pose beamPose(Vec3 start,Vec3 end,Vec3 viewer) {
    Vec3 y=unit(end-start),center=(start+end)*.5f;
    if(y.length()<.5f) return {};
    Vec3 x=unit(cross(y,viewer-center));
    if(x.length()<.5f) x=unit(cross(y,std::abs(y.y)<.9f?Vec3{0,1,0}:Vec3{1,0,0}));
    Vec3 z=cross(x,y);
    return {center,basisQuaternion(x,y,z),true};
}
}
