#include "adaptive_pose_filter.hpp"
#undef NDEBUG
#include <cassert>
#include <limits>
int main() {
 using namespace copvr;
 AdaptivePoseFilter filter; Pose raw{{0,0,0},{},true};
 assert(filter.update(raw,100).position.x==0);
 raw.position.x=.002f; auto filtered=filter.update(raw,110);
 assert(filtered.position.x>0&&filtered.position.x<raw.position.x);
 auto again=filter.update(raw,110); assert(again.position.x==filtered.position.x); // Multiple consumers.
 raw.position.x=.1f; filtered=filter.update(raw,120);
 assert((filtered.position-raw.position).length()<=.01501f);
 raw.orientation={0,std::sin(.2f),0,std::cos(.2f)};
 filtered=filter.update(raw,130); assert(poseQAngle(filtered.orientation,raw.orientation)<.044f);
 raw.orientation={-raw.orientation.x,-raw.orientation.y,-raw.orientation.z,-raw.orientation.w};
 filtered=filter.update(raw,140); assert(poseQAngle(filtered.orientation,raw.orientation)<.044f);
 raw.position.x=2;filtered=filter.update(raw,150);assert(filtered.position.x==2); // Recenter/teleport.
 raw.position.x=2.05f;filtered=filter.update(raw,400);assert(filtered.position.x==raw.position.x); // Tracking gap.
 raw.valid=false;assert(!filter.update(raw,410).valid);
 raw.valid=true;raw.position.x=3;assert(filter.update(raw,420).position.x==3); // Reacquisition snaps.
 float inputEnergy=0,outputEnergy=0;
 filter.reset(); raw.position={};raw.orientation={};filter.update(raw,1000);
 for(int i=1;i<=100;++i) {
  raw.position.x=i%2?.001f:-.001f;filtered=filter.update(raw,1000+i*10);
  inputEnergy+=raw.position.x*raw.position.x;outputEnergy+=filtered.position.x*filtered.position.x;
 }
 assert(outputEnergy<inputEnergy*.6f); // Stationary jitter reduced.
 // Invalid inputs must not poison future samples.
 raw.position.x=std::numeric_limits<float>::quiet_NaN();assert(!filter.update(raw,2100).valid);
 raw.position.x=0;assert(filter.update(raw,2110).valid);
}
