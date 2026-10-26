#include "vr_shadow_size.hpp"
#undef NDEBUG
#include <cassert>
int main(){
    copvr::FrameShadowSize a,b;
    assert(a.select(0,512)==512);
    assert(a.select(0,520)==512);
    assert(a.select(0,500)==512);
    assert(b.select(0,1024)==1024);
    assert(a.select(1,600)==600);
    assert(a.select(1,512)==600);
    assert(a.select(UINT32_MAX,700)==700);
    assert(a.select(0,800)==800);
}
