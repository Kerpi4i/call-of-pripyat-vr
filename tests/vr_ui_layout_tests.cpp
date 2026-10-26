#include "vr_ui_layout.hpp"
#include "ui_ray.hpp"
#undef NDEBUG
#include <cassert>
int main() {
    using namespace copvr;
    assert(uiPanelWidth==1.875f&&uiPanelHeight==1.125f&&uiPanelDistance==2.f);
    assert(uiAspectCorrection(1024,768)>.7999f&&uiAspectCorrection(1024,768)<.8001f);
    const Vec3 panel{0,0,-uiPanelDistance};
    const auto centre=panelHit({0,0,0},{0,0,-1},panel,{},uiPanelWidth,uiPanelHeight);
    assert(centre.valid&&centre.x==.5f&&centre.y==.5f);
    const auto top=panelHit({0,uiPanelHeight*.5f,0},{0,0,-1},panel,{},uiPanelWidth,uiPanelHeight);
    assert(top.valid&&top.y==0);
    assert(!panelHit({0,uiPanelHeight*.6f,0},{0,0,-1},panel,{},uiPanelWidth,uiPanelHeight).valid);
}
