#include "body_pose_estimator.hpp"
#undef NDEBUG
#include <cassert>
int main() {
 using namespace copvr;
 BodyPoseEstimator body;
 auto pose=body.update({0,0,0},{0,0,1},{0,1,0},0,1.65f,.01f);
 assert(pose.waistOrigin.length()<1e-5f);
 assert(std::abs(pose.chestCenter.y+.29f)<1e-5f);
 assert(std::abs(pose.chestCenter.z)<1e-5f);
 // Pure crouch lowers waist and chest together.
 pose=body.update({0,-.35f,0},{0,0,1},{0,1,0},0,1.65f,.01f);
 assert(std::abs(pose.waistOrigin.y+.35f)<1e-5f);
 assert(std::abs(pose.chestCenter.y+.64f)<1e-5f);
 // A bend around the hips does not drag the waist under the headset.
 const float spine=1.65f*.45f-.10f;
 const float bendDrop=spine-std::sqrt(spine*spine-.30f*.30f);
 pose=body.update({0,-bendDrop,.30f},{0,0,1},{0,1,0},0,1.65f,.01f);
 assert(std::abs(pose.waistOrigin.z)<1e-5f&&std::abs(pose.waistOrigin.y)<1e-5f);
 assert(pose.chestCenter.z>.15f&&pose.chestCenter.z<.30f);
 assert(pose.chestUp.z>0);
 // Looking down around the neck, without bending the spine.
 const Vec3 up{0,.70710678f,.70710678f},forward{0,-.70710678f,.70710678f};
 const Vec3 head=Vec3{0,-.10f,-.08f}+up*.10f+forward*.08f;
 pose=body.update(head,forward,up,0,1.65f,.01f);
 assert(pose.waistOrigin.length()<1e-4f);
 assert(std::abs(pose.chestCenter.z)<1e-4f);
 // Deep crouch keeps the estimated pelvis above the floor.
 pose=body.update({0,-1.f,0},{0,0,1},{0,1,0},0,1.65f,.01f);
 assert(1.65f+pose.pelvis.y>=.2999f);
 assert(pose.chestCenter.y-(pose.waistOrigin.y-.535f)>=.2299f);
 // Upright room-scale steps eventually move the body anchor.
 for(int i=0;i<200;++i)pose=body.update({.4f,0,0},{0,0,1},{0,1,0},0,1.65f,.01f);
 assert(pose.waistOrigin.x>.25f);
 pose=body.update({.4f,0,0},{0,0,1},{0,1,0},0,1.65f,.01f,true);
 assert(std::abs(pose.waistOrigin.x-.4f)<1e-5f);
}
