#include "projection_decompression.hpp"
#include <cmath>
#undef NDEBUG
#include <cassert>
#include <iostream>
int main() {
    for(float shift : {-.24f,0.f,.24f}) for(float tilt : {-.15f,0.f,.15f}) {
        const float p11=1.12f,p22=1.23f,width=1920,height=1080;
        auto d=copvr::decompression(p11,p22,shift,tilt,width,height);
        for(float depth : {.1f,1.f,30.f}) for(float x : {-.05f,0.f,.07f}) for(float y : {-.04f,0.f,.06f}) {
            float pixelX=(p11*x/depth+shift+1)*width*.5f;
            float pixelY=(1-p22*y/depth-tilt)*height*.5f;
            assert(std::abs((pixelX*d.z-d.x)*depth-x)<.00001f);
            assert(std::abs((pixelY*d.w-d.y)*depth-y)<.00001f);
        }
    }
    std::cout<<"Projection decompression tests passed\n";
}
