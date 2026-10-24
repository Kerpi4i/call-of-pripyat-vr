#pragma once
#include "interaction.hpp"
namespace copvr {
struct PanelHit { bool valid{}; float x{},y{},distance{}; };
inline PanelHit panelHit(Vec3 origin,Vec3 direction,Vec3 center,Quaternion rotation,float width,float height) {
    Quaternion inverse{-rotation.x,-rotation.y,-rotation.z,rotation.w};
    auto p=rotate(inverse,origin-center),d=rotate(inverse,direction);
    // Only the front face is interactive; OpenXR panel front is +Z.
    if(!std::isfinite(d.z)||d.z>=-0.0001f||width<=0||height<=0) return {};
    float t=-p.z/d.z;
    if(!std::isfinite(t)||t<0||t>10) return {};
    float x=(p.x+t*d.x)/width+.5f,y=.5f-(p.y+t*d.y)/height;
    if(!std::isfinite(x)||!std::isfinite(y)||x<0||x>1||y<0||y>1) return {};
    return {true,x,y,t};
}
}
