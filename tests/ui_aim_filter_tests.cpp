#include "ui_aim_filter.hpp"
#undef NDEBUG
#include <cassert>
int main() {
    using namespace copvr;
    UiAimFilter ui; ui.context(1);
    AdaptivePoseFilter gameplay;
    Pose raw{{0,0,0},{},true};
    ui.update(0,raw,1000); gameplay.update(raw,1000);
    float uiEnergy=0,gameEnergy=0;
    for(int i=1;i<=150;++i) {
        const float angle=i%2?.006f:-.006f;
        raw.orientation={0,std::sin(angle*.5f),0,std::cos(angle*.5f)};
        const auto a=ui.update(0,raw,1000+i*11);
        const auto b=gameplay.update(raw,1000+i*11);
        uiEnergy+=a.orientation.y*a.orientation.y;
        gameEnergy+=b.orientation.y*b.orientation.y;
    }
    assert(uiEnergy<gameEnergy*.4f); // Much steadier pointing at small targets.
    raw.position={.1f,0,0};raw.orientation={};
    const auto other=ui.update(1,raw,2700);
    assert(other.position.x==raw.position.x); // Hands never share history.
    ui.context(2);raw.position={.15f,0,0};
    assert(ui.update(0,raw,2711).position.x==raw.position.x); // Menu/overlay transition.
    raw.valid=false;assert(!ui.update(0,raw,2722).valid);
    raw.valid=true;raw.position={.17f,0,0};
    assert(ui.update(0,raw,2733).position.x==raw.position.x); // Tracking reacquisition.
    ui.context(0);assert(!ui.update(0,raw,2744).valid);
    ui.context(2);raw.position={.2f,0,0};
    assert(ui.update(0,raw,2755).position.x==raw.position.x); // Focus/UI re-entry.
    for(int i=1;i<=20;++i) {
        raw.position.x+=.008f;float angle=i*.04f;
        raw.orientation={0,std::sin(angle*.5f),0,std::cos(angle*.5f)};
        const auto filtered=ui.update(0,raw,2755+i*11);
        assert((filtered.position-raw.position).length()<=.02501f);
        assert(poseQAngle(filtered.orientation,raw.orientation)<.0525f);
    }
}
