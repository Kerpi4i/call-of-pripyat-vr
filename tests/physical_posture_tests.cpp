#include "physical_posture.hpp"
#include <cassert>
#include <limits>
int main(){
 copvr::PhysicalPostureGate gate;
 assert(!gate.update(true,0));assert(!gate.update(true,-.24f));assert(gate.update(true,-.30f));
 assert(gate.update(true,-.21f));assert(!gate.update(true,-.17f));
 assert(!copvr::canCalibrateStanding(-.30f));assert(!copvr::canCalibrateStanding(-.21f));assert(copvr::canCalibrateStanding(0));
 assert(copvr::canCalibrateStanding(-.18f));assert(!copvr::canCalibrateStanding(std::numeric_limits<float>::quiet_NaN()));
 gate.update(true,-.5f);assert(!gate.update(false,-.5f));assert(!gate.update(true,std::numeric_limits<float>::infinity()));
}
