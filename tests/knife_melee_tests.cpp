#include "knife_melee.hpp"
#undef NDEBUG
#include <cassert>
int main(){
 using namespace copvr;
 assert(knifeDamageScale(.5f)==0);
 assert(knifeDamageScale(1.f)<knifeDamageScale(3.f));
 assert(knifeDamageScale(6.f)==2.f&&knifeDamageScale(10.f)==2.f);
 KnifeSwingGate gate;
 assert(!gate.update({3,0,0},.02f)); // No equip strike.
 for(int i=0;i<5;++i) gate.update({0,0,0},.02f);
 assert(gate.update({3,0,0},.02f));
 gate.contact({3,0,0});
 for(int i=0;i<20;++i) assert(!gate.update({3,0,0},.02f)); // One contact per continuous swing.
 assert(gate.update({-3,0,0},.02f)); // New backswing.
 gate.contact({-3,0,0});
 assert(!gate.update({3,0,0},.02f)); // Cooldown.
 for(int i=0;i<12;++i) gate.update({0,0,0},.02f);
 assert(gate.update({2,0,0},.02f));
 assert(!gate.update({30,0,0},.02f)); // Tracking jump disarms.
 assert(!gate.update({3,0,0},.02f));
}
