#include "two_hand.hpp"
#undef NDEBUG
#include <cassert>
#include <iostream>
using namespace copvr;
int main() {
    SupportGrip g; Vec3 a{},b{0,0,.3f},aim{0,0,1};
    assert(!g.update(true,a,b,aim,1)); // release before acquire
    g.update(true,a,b,aim,0); assert(g.update(true,a,b,aim,1));
    assert(g.update(true,a,{.3f,0,.2f},aim,.5f)); // support can steer sideways
    assert(!g.update(true,a,b,aim,.1f));
    assert(!g.update(true,a,{0,0,-.3f},aim,1)); // no grabs behind
    assert(!g.update(true,a,b,aim,1)); // missed squeeze needs release
    g.update(true,a,b,aim,0); assert(g.update(true,a,b,aim,1));
    assert(!g.update(false,a,b,aim,1)); assert(!g.update(true,a,b,aim,1));
    g.update(true,a,b,aim,0); g.update(true,a,b,aim,1);
    assert(!g.update(true,a,{0,0,1},aim,1));
    g.reset(); assert(!g.active());
    std::cout<<"Two-hand support grip tests passed\n";
}
