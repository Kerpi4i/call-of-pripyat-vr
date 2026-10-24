#include "rain_step.hpp"
#undef NDEBUG
#include <cassert>
#include <iostream>
int main() {
    copvr::RainStep step; float traveled=0;
    for(unsigned frame=0;frame<90;++frame) {
        if(step.advance(frame)) traveled+=60.f/90.f;
        assert(!step.advance(frame));
        assert(!step.advance(frame)); // repeated view/render pass
    }
    assert(traveled>59.99f&&traveled<60.01f);
    assert(step.advance(0xffffffffu)); assert(!step.advance(0xffffffffu));
    assert(step.advance(0)); // frame counter wrap
    std::cout<<"PASS: rain simulation advances once per frame across stereo/repeated passes and frame wrap\n";
}
