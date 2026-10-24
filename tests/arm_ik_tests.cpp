#include "arm_ik.hpp"
#undef NDEBUG
#include <cassert>
#include <iostream>
using namespace copvr;
void check(Vec3 wrist) {
    auto arm=solveArm({0,0,0},wrist,{-.3f,-.5f,0},.3f,.25f);
    assert(arm.valid && (arm.wrist-wrist).length()<.0001f);
    assert(std::abs((arm.elbow-arm.shoulder).length()-.3f)<.0001f);
    assert(std::abs((arm.wrist-arm.elbow).length()-.25f)<.0001f);
}
int main() {
    check({0,0,.4f}); check({0,0,1}); check({}); check({.1f,-.2f,.2f});
    auto q=rotationBetween({0,0,1},{0,1,0});
    assert((rotate(q,{0,0,1})-Vec3{0,1,0}).length()<.0001f);
    q=rotationBetween({0,0,1},{0,0,-1});
    assert((rotate(q,{0,0,1})-Vec3{0,0,-1}).length()<.0001f);
    assert(!solveArm({},{},{},0,.3f).valid);
    std::cout<<"Arm IK tests passed\n";
}
