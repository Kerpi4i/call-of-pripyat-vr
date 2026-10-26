#include "vr_render_geometry.hpp"
#undef NDEBUG
#include <cassert>
#include <initializer_list>
struct Projection {
    float _11=1.2f,_22=1.1f,_31=.12f,_32=-.07f,_33=0,_34=1,_43=0;
};
int main() {
    using namespace copvr;
    assert(parseEyeSize("1920x1920 -nointro").height==1920);
    assert(parseEyeSize("1832x1920").width==1832);
    for(const char* bad:{"0x1920","1920x0","1919x99999","100x100","1920x1920garbage","foo"})
        assert(parseEyeSize(bad).width==0);
    Projection eye;eye._33=1000.f/(1000.f-.05f);eye._43=-.05f*eye._33;
    for(float farPlane:{20.f,80.f,180.f}) {
        Projection shadow;
        assert(projectionWithFar(shadow,eye,farPlane));
        assert(shadow._11==eye._11&&shadow._22==eye._22);
        assert(shadow._31==eye._31&&shadow._32==eye._32&&shadow._34==1);
        assert(std::abs((.05f*shadow._33+shadow._43)/.05f)<1e-5f);
        assert(std::abs((farPlane*shadow._33+shadow._43)/farPlane-1.f)<1e-5f);
        assert(std::abs(-shadow._43/shadow._33-.05f)<1e-6f);
    }
    Projection invalid=eye,out;invalid._33=0;
    assert(!projectionWithFar(out,invalid,20.f));
    assert(!projectionWithFar(out,eye,.01f));
    for(float offset:{-.35f,0.f,.35f}) {
        eye._31=offset;eye._32=-offset;
        Projection safe;
        assert(conservativeShadowProjection(safe,eye,.2f,180.f));
        assert(safe._31==0&&safe._32==0);
        for(float edge:{-1.f,1.f}) {
            const float rayX=(edge-eye._31)/eye._11;
            const float rayY=(edge-eye._32)/eye._22;
            assert(std::abs(rayX*safe._11)<1.f);
            assert(std::abs(rayY*safe._22)<1.f);
        }
        assert(std::abs((.2f*safe._33+safe._43)/.2f)<1e-5f);
        assert(std::abs((180.f*safe._33+safe._43)/180.f-1.f)<1e-5f);
    }
}
