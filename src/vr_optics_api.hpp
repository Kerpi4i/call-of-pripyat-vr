#pragma once
#include <stdint.h>
struct CopVrOpticsView { uint32_t abi; float position[3],direction[3],up[3],fov; };
using CopVrGetOptics=int(*)(CopVrOpticsView*);
// Positive view-query return values; structure ABI remains unchanged.
inline constexpr int COPVR_OPTICS_SCOPE=1;
inline constexpr int COPVR_OPTICS_BINOCULAR=2;
inline constexpr int COPVR_OPTICS_BINOCULAR_PASS_BIT=0x100;
