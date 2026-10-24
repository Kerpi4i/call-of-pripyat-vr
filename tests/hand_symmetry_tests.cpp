#include "hand_basis.hpp"
#undef NDEBUG
#include <cassert>
#include <iostream>
using namespace copvr;
int main() {
    Vec3 wrist{.2f,0,0},index{.17f,0,.08f},middle{.2f,0,.1f},little{.24f,0,.07f};
    auto r=palmBasis(wrist,index,middle,little,1);
    auto l=palmBasis(mirror(wrist),mirror(index),mirror(middle),mirror(little),0);
    assert(r.valid&&l.valid);
    assert((l.x-mirror(r.x)*-1).length()<.0001f);
    assert((l.y-mirror(r.y)).length()<.0001f);
    assert((l.z-mirror(r.z)).length()<.0001f);
    const float s=std::sqrt(.5f);
    Quaternion rightRoll{0,0,-s,s},leftRoll{0,0,s,s};
    for(auto controller : {Quaternion{},Quaternion{.2f,.3f,.1f,std::sqrt(.86f)},Quaternion{0,s,0,s}}) {
        Quaternion reflected{controller.x,-controller.y,-controller.z,controller.w};
        for(auto p : {index,middle,little}) {
            auto delta=p-wrist,deltaLeft=mirror(p)-mirror(wrist);
            Vec3 canonical{dot(r.x,delta),dot(r.y,delta),dot(r.z,delta)};
            Vec3 canonicalLeft{dot(l.x,deltaLeft),dot(l.y,deltaLeft),dot(l.z,deltaLeft)};
            auto mappedRight=rotate(controller,rotate(rightRoll,canonical));
            auto mappedLeft=rotate(reflected,rotate(leftRoll,canonicalLeft));
            assert((mappedLeft-mirror(mappedRight)).length()<.0001f);
        }
    }
    // Mirrored shoulder/target/pole produce mirrored solutions, including
    // unreachable and crossed-hand targets; no forced mirroring of input.
    for(Vec3 target : {Vec3{.35f,-.3f,.4f},Vec3{.9f,-.2f,.8f},Vec3{-.1f,-.25f,.1f}}) {
        auto right=solveArm({.18f,-.2f,0},target,{.48f,-.7f,-.15f},.3f,.26f);
        auto left=solveArm({-.18f,-.2f,0},mirror(target),{-.48f,-.7f,-.15f},.3f,.26f);
        assert((left.shoulder-mirror(right.shoulder)).length()<.0001f);
        assert((left.elbow-mirror(right.elbow)).length()<.0001f);
        assert((left.wrist-mirror(right.wrist)).length()<.0001f);
    }
    assert(!palmBasis({},{},{},{},0).valid);
    // Mirroring authored contact frames preserves rotation and mirrors thumb
    // geometry. A second mirror must recover the exact source orientation.
    for(auto rotation:{Quaternion{},Quaternion{0,s,0,s},Quaternion{s,0,0,s},Quaternion{.2f,.3f,.1f,std::sqrt(.86f)}}) {
        auto x=rotate(rotation,{1,0,0}),y=rotate(rotation,{0,1,0}),z=rotate(rotation,{0,0,1});
        auto mirrored=mirrorContactBasis(x,y,z);
        assert(std::abs(dot(cross(mirrored.x,mirrored.y),mirrored.z)-1.f)<.0001f);
        auto restored=mirrorContactBasis(mirrored.x,mirrored.y,mirrored.z);
        assert((restored.x-x).length()<.0001f&&(restored.y-y).length()<.0001f&&(restored.z-z).length()<.0001f);
        Vec3 thumb{.035f,.01f,.04f}; auto reflectedThumb=mirror(thumb);
        auto target=mirrored.x*reflectedThumb.x+mirrored.y*reflectedThumb.y+mirrored.z*reflectedThumb.z;
        assert((target-mirror(rotate(rotation,thumb))).length()<.0001f);
        // Opposite-side contact must reflect translation too. Copying the
        // right palm centre onto the left puts the handle on its dorsal side.
        Vec3 wrist{.045f,-.08f,.12f};
        auto leftWrist=mirrorContactPoint(wrist);
        auto leftPalm=leftWrist+target;
        assert((leftPalm-mirrorContactPoint(wrist+rotate(rotation,thumb))).length()<.0001f);
        assert((mirrorContactPoint(leftWrist)-wrist).length()<.0001f);
        assert(std::abs(leftWrist.x-wrist.x)>.08f);
    }
    std::cout<<"Hand symmetry tests passed\n";
}
