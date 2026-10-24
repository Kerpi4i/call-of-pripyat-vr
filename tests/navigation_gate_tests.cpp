#include "navigation_gate.hpp"
#undef NDEBUG
#include <cassert>
#include <limits>
#include <iostream>
using namespace copvr;
int main() {
    NavigationGate g; CopVrNavigation n{};
    n.moveY=1; n.buttons=COPVR_A;
    assert(g.update(true,n).held==0); // no movement on acquisition while held
    n={}; g.update(true,n); n.moveY=1; n.turnX=1; n.buttons=COPVR_A;
    auto e=g.update(true,n); assert(e.y==1&&e.snap==1&&e.pressed==COPVR_A);
    assert(g.update(true,n).snap==0&&g.update(true,n).pressed==0);
    e=g.update(false,n); assert(e.released==COPVR_A&&e.y==0);
    assert(g.update(true,n).held==0); // focus/menu/tracking rearm
    n={}; g.update(true,n); n.turnX=-1;
    assert(g.update(true,n).snap==-1); assert(g.update(true,n).snap==0);
    n.moveX=std::numeric_limits<float>::quiet_NaN(); assert(g.update(true,n).x==0);
    ButtonEdges b; assert(b.update(true,COPVR_X)==0);
    b.update(true,0); assert(b.update(true,COPVR_X)==COPVR_X);
    assert(b.update(true,COPVR_X)==0);
    b.update(false,0); assert(b.update(true,COPVR_X)==0);
    NavigationGate weapon; n={}; weapon.update(true,n);
    n.buttons=COPVR_HOLSTER; assert(weapon.update(true,n).pressed==COPVR_HOLSTER);
    assert(weapon.update(true,n).pressed==0);
    n.buttons=0; assert(weapon.update(true,n).released==COPVR_HOLSTER);
    n.buttons=COPVR_RELOAD; assert(weapon.update(true,n).pressed==COPVR_RELOAD);
    std::cout<<"Navigation gate tests passed\n";
}
