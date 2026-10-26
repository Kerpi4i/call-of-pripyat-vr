#include "vr_shadow_policy.hpp"
#include "vr_optics_api.hpp"
#undef NDEBUG
#include <cassert>
#include <initializer_list>
int main(){
    for(bool binocular:{false,true}) {
        assert(copvr::localShadowsDisabled("",binocular)==binocular);
        assert(copvr::sunShadowsDisabled("",binocular)==binocular);
        assert(!copvr::localShadowsDisabled("-copvr_binocular_shadows",binocular));
        assert(!copvr::sunShadowsDisabled("-copvr_binocular_shadows",binocular));
        assert(copvr::localShadowsDisabled("-copvr_no_dynamic_shadows",binocular));
        assert(copvr::sunShadowsDisabled("-copvr_no_dynamic_shadows",binocular));
    }
    assert(copvr::localShadowsDisabled("-copvr_no_local_shadows",false));
    assert(!copvr::sunShadowsDisabled("-copvr_no_local_shadows",false));
    assert(!copvr::localShadowsDisabled("-copvr_no_sun_shadows",false));
    assert(copvr::sunShadowsDisabled("-copvr_no_sun_shadows",false));
    static_assert(COPVR_OPTICS_SCOPE!=COPVR_OPTICS_BINOCULAR);
    for(int eye=1;eye<=2;++eye) {
        const int encoded=eye|COPVR_OPTICS_BINOCULAR_PASS_BIT;
        assert((encoded&0xff)==eye);
    }
}
