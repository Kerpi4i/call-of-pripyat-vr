#pragma once
#include "vr_shadow_policy.hpp"
#if defined(USE_DX11)
extern "C" int copvr_binocular_pass();
inline bool copvrBinocularPass(){return copvr_binocular_pass()!=0;}
#else
inline bool copvrBinocularPass(){return false;}
#endif
inline bool copvrNoLocalShadows(const char* params){return copvr::localShadowsDisabled(params,copvrBinocularPass());}
inline bool copvrNoSunShadows(const char* params){return copvr::sunShadowsDisabled(params,copvrBinocularPass());}
