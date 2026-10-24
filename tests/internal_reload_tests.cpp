#include "internal_reload.hpp"
#include <cassert>
int main() {
    using namespace copvr;
    InternalReload tube;tube.init(7,InternalKind::Tube);
    assert(tube.canFire(InternalKind::Tube));tube.tubeLoaded(7);
    assert(tube.canFire(InternalKind::Tube)); // Topping up a chambered gun is interruptible.
    tube.shot(InternalKind::Tube);
    assert(!tube.canFire(InternalKind::Tube));assert(!tube.pumpTo(.5f,6));
    assert(!tube.pumpTo(0,6)); // Short stroke cannot prepare the next shot.
    assert(!tube.pumpTo(1,6));assert(tube.eject()==1);assert(!tube.eject());
    assert(tube.pumpTo(0,6));assert(tube.canFire(InternalKind::Tube));
    InternalReload first;first.init(0,InternalKind::Tube);first.tubeLoaded(0);
    assert(!first.canFire(InternalKind::Tube));assert(!first.pumpTo(1,1));
    assert(first.pumpTo(0,1));assert(first.canFire(InternalKind::Tube));
    InternalReload rotary;rotary.init(4,InternalKind::Rotary);
    rotary.tubeLoaded(4);assert(rotary.canFire(InternalKind::Rotary));
    rotary.shot(InternalKind::Rotary);assert(rotary.canFire(InternalKind::Rotary));
    rotary.tubeLoaded(0);assert(!rotary.canFire(InternalKind::Rotary));
    rotary.tubeLoaded(1);assert(!rotary.canFire(InternalKind::Rotary));
    rotary.pumpTo(.5f,2);assert(!rotary.pumpTo(0,2));assert(!rotary.canFire(InternalKind::Rotary));
    rotary.pumpTo(1,2);assert(rotary.pumpTo(0,2));assert(rotary.canFire(InternalKind::Rotary));
    rotary.tubeLoaded(2);assert(rotary.canFire(InternalKind::Rotary));
    rotary.shot(InternalKind::Rotary);assert(rotary.canFire(InternalKind::Rotary));
    InternalReload spas;spas.init(3,InternalKind::Tube);
    assert(!spas.canLoadTube(true));assert(spas.canLoadTube(false));
    spas.pumpTo(.5f,3);assert(!spas.canLoadTube(true));
    spas.pumpTo(1,3);assert(spas.canLoadTube(true));assert(!spas.canFire(InternalKind::Tube));
    spas.tubeLoaded(3);assert(spas.canLoadTube(true));
    spas.pumpTo(0,4);assert(!spas.canLoadTube(true));assert(spas.canFire(InternalKind::Tube));
    InternalReload drum;drum.init(0,InternalKind::Drum);
    assert(!drum.canFire(InternalKind::Drum));assert(!drum.drumLoad());
    drum.drumToggle();drum.angle=.3f;assert(!drum.drumLoad());
    drum.angle=.65f;
    for(unsigned i=0;i<6;++i){assert(drum.cursor==i);assert(drum.drumLoad());assert(!drum.drumIndexed());assert(!drum.drumLoad());drum.drumAnimate(1.f);assert(drum.drumIndexed());}
    assert(drum.live==63);assert(!drum.drumLoad());
    drum.drumToggle();assert(!drum.canFire(InternalKind::Drum));drum.angle=0;drum.drumAnimate(1.f);
    assert(drum.canFire(InternalKind::Drum));const auto firstChamber=drum.cursor;drum.shot(InternalKind::Drum);
    assert(!(drum.live&(1u<<firstChamber)));assert(!drum.canFire(InternalKind::Drum));
    const auto startPhase=drum.pump;drum.drumAnimate(.04f);assert(drum.pump!=startPhase);assert(!drum.drumIndexed());
    drum.drumAnimate(1.f);assert(drum.canFire(InternalKind::Drum));
    drum.drumToggle();drum.angle=.65f;assert(drum.cursor==firstChamber);drum.drumAnimate(1.f);
    assert(drum.drumLoad());assert(drum.live==63);drum.drumAnimate(1.f);
    drum.drumToggle();drum.angle=0;drum.drumAnimate(1.f);
    for(int i=0;i<6;++i){assert(drum.canFire(InternalKind::Drum));drum.shot(InternalKind::Drum);assert(!drum.drumIndexed());drum.drumAnimate(1.f);}
    assert(!drum.live&&!drum.canFire(InternalKind::Drum));
    drum.drumToggle();drum.angle=.65f;assert(drum.drumLoad());drum.drumAnimate(1.f);assert(drum.drumLoad());drum.drumAnimate(1.f);
    const unsigned partial=drum.live;drum.drumToggle();drum.angle=0;drum.drumAnimate(1.f);assert(drum.canFire(InternalKind::Drum));
    drum.drumToggle();drum.angle=.65f;assert(drum.live==partial);assert(!drum.canFire(InternalKind::Drum));
    InternalReload barrels;barrels.init(2,InternalKind::Break);
    barrels.shot(InternalKind::Break);assert(barrels.live==2&&barrels.spent==1);
    barrels.angle=.65f;assert(!barrels.canFire(InternalKind::Break));
    assert(!barrels.canLoad(0));assert(barrels.eject()==1);
    assert(barrels.load(0));assert(!barrels.load(0));assert(!barrels.load(1));
    barrels.angle=0;assert(barrels.canFire(InternalKind::Break));
    InternalReload empty;empty.init(0,InternalKind::Break);empty.angle=.65f;
    assert(empty.load(1));assert(empty.live==2);empty.angle=0;
    empty.shot(InternalKind::Break);assert(empty.live==0&&empty.spent==2);
    assert(empty.canFire(InternalKind::Rocket));
}
