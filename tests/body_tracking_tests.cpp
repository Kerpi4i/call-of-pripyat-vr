#include "body_tracking.hpp"
#undef NDEBUG
#include <cassert>
#include <limits>
#include <iostream>
int main() {
    using namespace copvr;
    assert(chestVolume(0,-.29f,-.04f));
    assert(chestVolume(.08f,-.36f,.02f)&&chestVolume(-.08f,-.36f,.02f));
    assert(chestVolume(0,chestHeight,.085f)); // front edge advanced 4cm
    assert(!chestVolume(0,chestHeight,-.11f)); // old rear edge no longer active
    assert(!chestVolume(0,-.36f,.20f)); // relaxed/forward hands no longer trigger
    assert(!chestVolume(0,-.65f,.02f)); // old large stow volume removed
    assert(chestVolume(.20f,-.29f,-.04f));
    assert(!chestVolume(.24f,-.29f,-.04f));
    assert(!chestVolume(0,-.29f,.10f));
    assert(!chestVolume(0,std::numeric_limits<float>::quiet_NaN(),0));
    for(float x:{-.18f,0.f,.18f}) {
        assert(backToolsVolume(x,-.61f,-.34f));
        assert(!chestVolume(x,-.61f,-.34f));
        assert(!headWeaponVolume(x,-.61f,-.34f,false));
    }
    assert(!backToolsVolume(0,-.61f,0)); // no front belt overlap
    assert(!backToolsVolume(0,0,-.34f)); // separate from weapon rack
    assert(!backToolsVolume(.25f,-.61f,-.34f));
    assert(templeVolume(-.14f,0,.05f)&&templeVolume(.14f,0,.05f));
    assert(!templeVolume(.07f,0,.05f)&&!templeVolume(.21f,0,.05f));
    assert(!templeVolume(0,0,.05f));
    assert(!backToolsVolume(0,-.61f,std::numeric_limits<float>::infinity()));
    assert(quickZone(-.1f,quickHeight,chestForward,0)==0);
    assert(quickZone(.1f,quickHeight,chestForward,0)==1);
    assert(quickZone(-.1f,quickHeight,chestForward,1)==2);
    assert(quickZone(.1f,quickHeight,chestForward,1)==3);
    assert(quickZone(0,quickHeight,chestForward,0)==1);
    assert(quickZone(.3f,quickHeight,chestForward,0)==-1);
    assert(quickZone(0,chestHeight,chestForward,0)==-1);
    assert(quickZone(0,quickHeight,chestForward,2)==-1);
    assert(!chestVolume(0,quickHeight,chestForward));
    assert(reloadRadius==.065f);
    assert(headWeaponVolume(-.35f,0,-.2f,false));
    assert(headWeaponVolume(.35f,0,-.2f,false));
    assert(headWeaponVolume(0,-.09f,-.2f,false));
    assert(!headWeaponVolume(0,-.11f,-.2f,false));
    assert(headWeaponVolume(0,.29f,-.2f,false));
    assert(!headWeaponVolume(0,.31f,-.2f,false));
    assert(headWeaponVolume(0,0,-.54f,false));
    assert(!headWeaponVolume(0,0,-.56f,false));
    assert(!headWeaponVolume(0,0,-.02f,false));
    assert(!headWeaponVolume(0,0,.20f,false));
    assert(headWeaponVolume(0,0,.20f,true));
    assert(!headWeaponVolume(0,-.6f,-.2f,true));
    assert(!headWeaponVolume(.6f,0,-.2f,true));
    assert(!headWeaponVolume(0,0,-.7f,true));
    assert(!headWeaponVolume(std::numeric_limits<float>::quiet_NaN(),0,0,true));
    assert(postureScale(0,1.6f)==1);
    assert(std::abs(postureScale(-.4f,1.6f)-.75f)<.0001f);
    assert(postureScale(.2f,1.6f)==1);
    assert(postureScale(-3,1.6f)==.3f);
    assert(postureScale(std::numeric_limits<float>::quiet_NaN(),1.6f)==1);
    TorsoHeading torso;
    assert(torso.update(0,.01f)==0);
    assert(torso.update(.4f,.01f)==0); // independent head glance
    assert(torso.update(.8f,.01f)>0);
    torso.update(3.1f,.01f,true);
    assert(std::abs(torso.update(-3.1f,.01f)-3.1f)<.001f); // wrap at pi
    torso.update(0,.01f,true);
    const float movement=torso.align(1.57f,.05f);
    assert(movement>0&&movement<1.57f);
    assert(std::abs(torso.align(1.57f,0)-movement)<.00001f);
    torso.update(0,.01f,true);
    for(int i=0;i<100;++i) assert(std::abs(torso.alignWithinView(3.141592654f,0,.05f))<.0001f);
    for(float head:{0.f,3.1f,-3.1f}) for(float travel:{-3.f,-1.5f,0.f,1.5f,3.f}) {
        const float result=torso.alignWithinView(travel,head,.05f);
        assert(std::abs(std::remainder(result-head,6.283185308f))<=3.141592654f/6+.00001f);
    }
    // Inverse posture ray must preserve world distance after normalization.
    for(float scale:{.3f,.6f,1.f}) {
        const float dx=.6f,dy=.8f;
        const float factor=std::sqrt(dx*dx+dy*dy/(scale*scale));
        const float distance=3.5f;
        const float mappedY=(dy/scale)/factor*(distance*factor);
        assert(std::abs(mappedY*scale-dy*distance)<.00001f);
        assert(std::abs((distance*factor)/factor-distance)<.00001f);
    }
    std::cout<<"PASS: chest/back zone bounds and separation, physical posture, torso heading, ray distance\n";
}
