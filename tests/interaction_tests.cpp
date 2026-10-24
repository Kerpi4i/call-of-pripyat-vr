#include "interaction.hpp"
#include <iostream>
#include <cstdlib>
using namespace copvr;
void require(bool condition,const char* message) { if(!condition) { std::cerr<<message<<'\n'; std::exit(1); } }
int main() {
    Interaction logic; Frame f; f.focused=true; f.head.valid=true;
    for(auto& h:f.hands) { h.grip.valid=true; h.aim.valid=true; }
    f.hands[1].grip.position={3,0,0};
    std::vector<Item> items{{7,{{0,0,0},{},true},Location::World,-1,true}};
    std::vector<Slot> slots{{{0,1,0}}};
    f.hands[0].squeeze=1;
    auto e=logic.update(f,items,slots);
    require(e.size()==1&&e[0].type==EventType::Grab,"grab nearest weapon");
    f.hands[0].trigger=1;
    e=logic.update(f,items,slots);
    require(e.size()==1&&e[0].type==EventType::FireStart,"trigger starts fire");
    require(logic.update(f,items,slots).empty(),"held trigger must not repeat start");
    f.focused=false; e=logic.update(f,items,slots);
    require(e.size()==1&&e[0].type==EventType::FireStop,"focus loss stops fire");
    f.focused=true;
    require(logic.update(f,items,slots).empty(),"focus return must not auto fire");
    f.hands[0].trigger=0; logic.update(f,items,slots);
    f.hands[0].grip.position={0,1,0}; f.hands[0].squeeze=0;
    e=logic.update(f,items,slots);
    require(e.size()==1&&e[0].type==EventType::Stow&&slots[0].item==7,"stow in nearby slot");
    f.hands[0].squeeze=1; e=logic.update(f,items,slots);
    require(e.size()==1&&e[0].type==EventType::Grab&&!slots[0].item,"draw from slot");
    f.hands[0].grip.valid=false; f.hands[0].squeeze=0;
    require(logic.update(f,items,slots).empty()&&logic.held()[0]==7,"tracking loss keeps item");
    f.hands[0].grip.valid=true; f.hands[0].grip.position={1,0,0};
    f.hands[0].velocityValid=true; f.hands[0].velocity={2,0,0};
    e=logic.update(f,items,slots);
    require(e.size()==1&&e[0].type==EventType::Drop&&e[0].velocity.x==2,"drop carries tracked velocity");
    auto direction=to_xray(rotate({}, {0,0,-1}));
    require(direction.z==1,"forward conversion");
    Interaction ownership; Frame both; both.focused=true; both.head.valid=true;
    for(auto& h:both.hands) { h.grip.valid=true; h.aim.valid=true; h.squeeze=1; }
    std::vector<Item> shared{{1,{{0,0,0},{},true}}};
    std::vector<Slot> noSlots;
    e=ownership.update(both,shared,noSlots);
    require(e.size()==1&&ownership.held()[0]==1&&!ownership.held()[1],"two hands cannot own same item");
    both.hands[0].squeeze=0.4f;
    require(ownership.update(both,shared,noSlots).empty(),"grip hysteresis keeps item");
    Interaction occupied; std::vector<Slot> full{{{0,0,0},0.16f,9}};
    std::vector<Item> objects{{2,{{0,0,0},{},true}},{9,{{0,0,0},{},true},Location::Slot,0}};
    both.hands[0].squeeze=1; both.hands[1].grip.position={4,0,0};
    occupied.update(both,objects,full); both.hands[0].squeeze=0;
    e=occupied.update(both,objects,full);
    require(e.size()==1&&e[0].type==EventType::Drop&&full[0].item==9,"occupied slot must not overwrite inventory");
    Interaction safety; Frame unsafe; unsafe.focused=true; unsafe.head.valid=true;
    unsafe.hands[0].grip.valid=true; unsafe.hands[0].squeeze=1; unsafe.hands[0].trigger=1;
    std::vector<Item> gun{{3,{{0,0,0},{},true},Location::World,-1,true}};
    e=safety.update(unsafe,gun,noSlots);
    require(e.size()==1&&e[0].type==EventType::Grab,"startup with held trigger cannot fire");
    unsafe.hands[0].trigger=0; safety.update(unsafe,gun,noSlots);
    unsafe.hands[0].trigger=1;
    require(safety.update(unsafe,gun,noSlots).empty(),"invalid aim prevents firing");
    unsafe.hands[0].aim.valid=true;
    e=safety.update(unsafe,gun,noSlots);
    require(e.size()==1&&e[0].type==EventType::FireStart,"valid aim resumes trigger action");
    unsafe.hands[0].grip.valid=false; e=safety.update(unsafe,gun,noSlots);
    require(e.size()==1&&e[0].type==EventType::FireStop,"tracking loss stops active weapon");
    std::cout<<"PASS: grab, fire, focus loss, trigger rearm, slots, tracking loss, throw, coordinate conversion, ownership, hysteresis, occupied slot, startup safety, aim validity\n";
}
