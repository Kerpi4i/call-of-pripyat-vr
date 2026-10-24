#include "stereo_math.hpp"
#include <cstdlib>
#include <iostream>
using namespace copvr;
void expect(bool value,const char* message) { if(!value) { std::cerr<<message<<'\n'; std::exit(1); } }
bool close(float a,float b) { return std::abs(a-b)<0.0001f; }
int main() {
    Pose eye{{0,0,0},{},true}; Frustum f{-.7f,.8f,.6f,-.5f};
    auto left=project({std::tan(f.left),0,-1},eye,f);
    auto right=project({std::tan(f.right),0,-1},eye,f);
    auto top=project({0,std::tan(f.up),-1},eye,f);
    auto bottom=project({0,std::tan(f.down),-1},eye,f);
    expect(close(left.x/left.w,-1)&&close(right.x/right.w,1),"asymmetric horizontal bounds");
    expect(close(top.y/top.w,1)&&close(bottom.y/bottom.w,-1),"asymmetric vertical bounds");
    auto near=project({0,0,-.05f},eye,f),far=project({0,0,-100},eye,f);
    expect(close(near.z/near.w,0)&&close(far.z/far.w,1),"D3D depth range");
    eye.position={2,1,4};
    auto shifted=project({2,1,3},eye,f), origin=project({0,0,-1},{{},{},true},f);
    expect(close(shifted.x,origin.x)&&close(shifted.y,origin.y),"inverse eye translation");
    eye={{},{0,.70710678f,0,.70710678f},true};
    auto turned=project({-1,0,0},eye,f);
    expect(close(turned.x,origin.x)&&close(turned.w,1),"inverse head rotation");
    Pose leftEye{{-.032f,0,0},{},true},rightEye{{.032f,0,0},{},true};
    auto a=project({0,0,-1},leftEye,{-.7f,.7f,.7f,-.7f});
    auto b=project({0,0,-1},rightEye,{-.7f,.7f,.7f,-.7f});
    expect(a.x>0&&b.x<0,"stereo parallax");
    std::cout<<"PASS: asymmetric FOV, depth, eye translation, head rotation, stereo parallax\n";
}
