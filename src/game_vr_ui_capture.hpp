#pragma once
namespace copvr_ui {
struct Capture {
    int mode{}; using Begin=int(*)(); using End=void(*)(); End end{};
    Capture() {
#if defined(XR_PLATFORM_WINDOWS)
        if(!strstr(Core.Params,"-copvr")) return;
        auto library=GetModuleHandleA("xrRender_R4.dll");
        auto begin=library?reinterpret_cast<Begin>(GetProcAddress(library,"copvr_ui_begin")):nullptr;
        end=library?reinterpret_cast<End>(GetProcAddress(library,"copvr_ui_end")):nullptr;
        if(begin&&end) mode=begin();
#endif
    }
    bool skip() const { return mode==2; }
    ~Capture() { if(mode==1&&end) end(); }
};
}
