#pragma once
namespace copvr {
struct Decompression { float x,y,z,w; };
// D3D pixel -> left-handed view ray for an off-centre projection.
inline Decompression decompression(float p11,float p22,float p31,float p32,float width,float height) {
    return {(1.f+p31)/p11,(p32-1.f)/p22,2.f/(width*p11),-2.f/(height*p22)};
}
}
