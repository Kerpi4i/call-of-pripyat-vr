#include "ui_ray.hpp"
#undef NDEBUG
#include <cassert>
#include <iostream>
using namespace copvr;
int main() {
    auto hit=panelHit({0,0,0},{0,0,-1},{0,0,-2},{},2,1);
    assert(hit.valid && hit.x==.5f && hit.y==.5f);
    hit=panelHit({.5f,.25f,0},{0,0,-1},{0,0,-2},{},2,1);
    assert(hit.valid && hit.x==.75f && hit.y==.25f);
    assert(!panelHit({2,0,0},{0,0,-1},{0,0,-2},{},2,1).valid);
    assert(!panelHit({0,0,-3},{0,0,1},{0,0,-2},{},2,1).valid);
    assert(!panelHit({0,0,0},{1,0,0},{0,0,-2},{},2,1).valid);
    float s=std::sqrt(.5f);
    hit=panelHit({0,0,0},{-1,0,0},{-2,0,0},{0,s,0,s},2,1);
    assert(hit.valid && std::abs(hit.x-.5f)<.001f);
    std::cout<<"UI panel ray tests passed\n";
}
