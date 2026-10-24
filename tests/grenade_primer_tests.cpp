#include "grenade_primer.hpp"
#undef NDEBUG
#include <cassert>
#include <iostream>
using namespace copvr;
int main() {
    GrenadePrimer trigger;
    assert(!trigger.update(1,1,false,0,{}));
    assert(!trigger.update(1,0,false,0,{}));
    assert(trigger.update(1,1,false,0,{}));
    assert(!trigger.update(1,1,false,0,{}));
    GrenadePrimer pin;
    assert(!pin.update(1,0,true,1,{.1f,0,0}));
    assert(!pin.update(1,0,true,0,{.1f,0,0}));
    assert(!pin.update(1,0,true,1,{.1f,0,0}));
    assert(!pin.update(1,0,true,1,{.14f,0,0}));
    assert(pin.update(1,0,true,1,{.21f,0,0}));
    assert(!pin.update(1,0,true,1,{.4f,0,0}));
    GrenadePrimer far;
    far.update(1,0,true,0,{});
    assert(!far.update(1,0,true,1,{.3f,0,0}));
    assert(!far.update(1,0,true,1,{.6f,0,0}));
    GrenadePrimer occupied;
    occupied.update(1,0,true,0,{}); occupied.update(1,0,true,1,{.1f,0,0});
    assert(!occupied.update(1,0,false,1,{.3f,0,0}));
    assert(!occupied.update(1,0,true,1,{.4f,0,0}));
    GrenadePrimer unheld;
    unheld.update(0,0,false,0,{});
    assert(!unheld.update(0,1,false,0,{}));
    std::cout<<"PASS: grenade fresh trigger, physical pin pull, occupied/tracking loss, duplicate and unheld inhibition\n";
}
