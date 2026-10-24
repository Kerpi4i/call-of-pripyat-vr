#include "beam_math.hpp"
#undef NDEBUG
#include <cassert>
#include <iostream>
using namespace copvr;
void check(Vec3 start,Vec3 end,Vec3 eye) {
    auto b=beamPose(start,end,eye); assert(b.valid);
    auto along=rotate(b.orientation,{0,1,0});
    assert((along-unit(end-start)).length()<.001f);
    auto normal=rotate(b.orientation,{0,0,1});
    assert(dot(normal,eye-b.position)>-.001f);
    assert(std::abs(dot(along,normal))<.001f);
    assert(std::abs(normal.length()-1)<.001f);
    assert((b.position-(start+end)*.5f).length()<.001f);
}
int main() {
    check({.3f,-.3f,0},{0,0,-2},{0,0,0});
    check({0,0,0},{0,1,0},{0,0,1});
    check({0,0,0},{0,-1,0},{0,0,-1});
    check({0,0,0},{1,0,0},{0,1,0});
    check({0,0,0},{-1,0,0},{0,-1,0});
    check({0,0,0},{0,0,-2},{0,0,0});
    assert(!beamPose({},{},{0,0,1}).valid);
    std::cout<<"Beam pose tests passed\n";
}
