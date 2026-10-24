#pragma once
#include "arm_ik.hpp"
namespace copvr {
struct PalmBasis { Vec3 x,y,z; bool valid{}; };
inline PalmBasis palmBasis(Vec3 wrist,Vec3 index,Vec3 middle,Vec3 little,int side) {
    Vec3 z=unit(middle-wrist),across=side==0?index-little:little-index;
    Vec3 y=unit(cross(z,across)),x=unit(cross(y,z));
    return {x,y,z,x.length()>.9f&&y.length()>.9f&&z.length()>.9f};
}
inline Vec3 mirror(Vec3 v) { return {-v.x,v.y,v.z}; }
inline Vec3 mirrorContactPoint(Vec3 v,float plane=0.f) { return {2.f*plane-v.x,v.y,v.z}; }
// Reflect a pose on both sides: S * R * S retains a proper rotation.
// Reflecting only R produces a negative scale and reverses the mesh winding.
struct MirroredBasis { Vec3 x,y,z; };
inline MirroredBasis mirrorContactBasis(Vec3 x,Vec3 y,Vec3 z) {
    return {mirror(x)*-1.f,mirror(y),mirror(z)};
}
}
