#include "vr_frame_mode.hpp"
#undef NDEBUG
#include <cassert>
#include <iostream>
int main() {
    using copvr::frameMode;
    assert(frameMode(true,false,false)==1); // reload with an existing level
    assert(frameMode(true,false,true)==1); // death caption cannot override loading
    assert(frameMode(true,true,true)==1);
    assert(frameMode(false,true,false)==1); // menu/intro
    assert(frameMode(false,false,true)==2); // inventory over world
    assert(frameMode(false,false,false)==0); // completed load enters stereo
    assert(frameMode(false,false,2)==3); // sleep caption does not capture gameplay
    assert(frameMode(true,false,2)==1);
    assert(frameMode(false,true,2)==1);
    std::cout<<"PASS: loading panel priority, death/reload, menu, world UI and gameplay modes\n";
}
