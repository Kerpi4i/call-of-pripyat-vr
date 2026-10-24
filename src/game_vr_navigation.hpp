#pragma once
#include "navigation_gate.hpp"
namespace copvr_game {
inline bool navigation(CopVrNavigation& value) {
    value={}; value.abi=COPVR_NAV_ABI;
    auto library=GetModuleHandleA("copvr_bridge.dll");
    auto sample=library?reinterpret_cast<CopVrGetNavigation>(GetProcAddress(library,"copvr_get_navigation")):nullptr;
    return sample&&sample(&value)>0;
}
}
