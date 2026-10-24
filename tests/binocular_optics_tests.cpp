#include "binocular_optics.hpp"
#undef NDEBUG
#include <cassert>
#include <limits>
int main() {
    using namespace copvr;
    assert(binocularProximity(.15f)==0.f);
    assert(std::abs(.02f*binocularProximity(.09f)-.01f)<1e-6f);
    assert(std::abs(.02f*binocularProximity(.03f)-.02f)<1e-6f);
    const auto tubeCentre=scopeTubeCoordinates(0,0,0,0,.08f,.013f);
    assert(std::abs(tubeCentre.x)<1e-6f&&std::abs(tubeCentre.y)<1e-6f);
    const auto tubeLeft=scopeTubeCoordinates(0,0,.02f,0,.08f,.013f);
    const auto tubeRight=scopeTubeCoordinates(0,0,-.02f,0,.08f,.013f);
    assert(tubeLeft.x<0&&std::abs(tubeLeft.x+tubeRight.x)<1e-6f);
    assert(scopeTubeCoordinates(.013f,0,0,0,.08f,.013f).x>1.f); // Visible black rim.
    assert(scopeTubeCoordinates(0,0,0,0,0,.013f).x==8.f);
    const float baseRadius=.0195f;
    assert(std::abs(binocularApertureRadius(baseRadius,.15f)-baseRadius)<1e-6f);
    assert(std::abs(binocularApertureRadius(baseRadius,.30f)-baseRadius)<1e-6f);
    assert(std::abs(binocularApertureRadius(baseRadius,.09f)-(baseRadius+.005f))<1e-6f);
    assert(std::abs(binocularApertureRadius(baseRadius,.03f)-(baseRadius+.01f))<1e-6f);
    assert(std::abs(binocularApertureRadius(baseRadius,0)-(baseRadius+.01f))<1e-6f);
    assert(binocularApertureRadius(baseRadius,std::numeric_limits<float>::quiet_NaN())==baseRadius);
    assert(headsetNearPlane<minimumOpticalPlane);
    assert(opticalPlaneSetback(.06f)==0);
    assert(std::abs(opticalPlaneSetback(.003f)-.052f)<1e-6f);
    assert(std::abs(opticalPlaneSetback(-.002f)-.057f)<1e-6f);
    assert(binocularNear({0,0,0},{0,0,.003f},{0,0,1},{0,0,1}));
    assert(binocularEyeLight({0,0,0},{0,0,.003f},{0,0,1})==1.f);
    const auto commonCentre=binocularSharedMask(.5f,.5f,.025f,.07f,0,0);
    assert(std::abs(commonCentre.x)<1e-6f&&std::abs(commonCentre.y)<1e-6f);
    const auto commonShift=binocularSharedMask(.5f,.5f,.025f,.07f,.02f,-.02f);
    assert(commonShift.x>0&&commonShift.y>0);
    const auto commonEdge=binocularSharedMask(0,0,.025f,.07f,0,0);
    assert(commonEdge.length()>1.f);
    BinocularZoom zoom;
    assert(zoom.update(true,0,1000)==1.f);
    assert(zoom.update(true,1,1050)>1.f);
    for(uint64_t t=1100;t<10000;t+=50) zoom.update(true,1,t);
    assert(zoom.update(true,1,10000)==1.5f);
    for(uint64_t t=10050;t<20000;t+=50) zoom.update(true,-1,t);
    assert(zoom.update(true,-1,20000)==.5f);
    assert(zoom.update(false,1,20050)==.5f);
    assert(zoom.update(true,1,20100)==.5f); // Neutral required after lost focus.

    // Corrected model basis: eye at +Z, camera looks toward -Z.
    assert(binocularNear({0,0,.12f},{0,0,0},{0,0,-1},{0,0,-1}));
    assert(!binocularNear({0,0,-.12f},{0,0,0},{0,0,-1},{0,0,-1}));
    assert(binocularEyeLight({0,0,.12f},{0,0,0},{0,0,-1})==1);
    assert(binocularNear({0,0,0},{0,0,.12f},{0,0,1},{0,0,1}));
    assert(!binocularNear({0,0,0},{0,0,.5f},{0,0,1},{0,0,1}));
    assert(!binocularNear({0,0,.2f},{0,0,.12f},{0,0,1},{0,0,1}));
    assert(!binocularNear({0,0,0},{0,0,.12f},{0,0,1},{1,0,0}));
    assert(!binocularNear({.2f,0,0},{0,0,.12f},{0,0,1},{0,0,1}));
    assert(binocularEyeLight({0,0,0},{0,0,.12f},{0,0,1})==1);
    assert(binocularEyeLight({0,0,.2f},{0,0,.12f},{0,0,1})==0);
    assert(binocularEyeLight({.1f,0,0},{0,0,.12f},{0,0,1})==0);
    assert(!binocularNear({0,0,0},{0,0,std::numeric_limits<float>::quiet_NaN()},{0,0,1},{0,0,1}));
}
