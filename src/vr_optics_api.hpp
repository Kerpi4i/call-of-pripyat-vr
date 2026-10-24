#pragma once
#include <stdint.h>
struct CopVrOpticsView { uint32_t abi; float position[3],direction[3],up[3],fov; };
using CopVrGetOptics=int(*)(CopVrOpticsView*);
