#pragma once
namespace copvr_game {
inline bool opticsPass() {
#if defined(XR_PLATFORM_WINDOWS)
    auto renderer=GetModuleHandleA("xrRender_R4.dll");
    using Query=int(*)();
    auto get=renderer?reinterpret_cast<Query>(GetProcAddress(renderer,"copvr_optics_pass")):nullptr;
    return get&&get()!=0;
#else
    return false;
#endif
}
}
