#pragma once
#include <cstring>
namespace copvr {
inline bool binocularShadowsDisabled(const char* params,bool binocularPass) {
    return binocularPass&&!std::strstr(params,"-copvr_binocular_shadows");
}
inline bool localShadowsDisabled(const char* params,bool binocularPass) {
    return std::strstr(params,"-copvr_no_dynamic_shadows")||std::strstr(params,"-copvr_no_local_shadows")||
        binocularShadowsDisabled(params,binocularPass);
}
inline bool sunShadowsDisabled(const char* params,bool binocularPass) {
    return std::strstr(params,"-copvr_no_dynamic_shadows")||std::strstr(params,"-copvr_no_sun_shadows")||
        binocularShadowsDisabled(params,binocularPass);
}
}
