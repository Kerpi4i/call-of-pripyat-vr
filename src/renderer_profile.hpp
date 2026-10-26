#pragma once
#include "vr_profile.hpp"
namespace copvr {
inline ProfileTrace& rendererTrace(){static ProfileTrace trace("renderer");return trace;}
class RendererProfilePass {
    using Marker=void(__cdecl*)(const char*,int);
    ProfileTrace& trace_;
    std::string name_;
    uint64_t frame_{}; double start_{}; Marker marker_{};
public:
    RendererProfilePass(const char* stage,int view,uint64_t frame,bool gpu=true):trace_(rendererTrace()),frame_(frame) {
        if(!trace_.enabled())return;
        const char* names[]={"eye0.","eye1.","optic0.","optic1."};
        name_=std::string(view>=0&&view<4?names[view]:"other.")+stage;
        if(gpu) {
            const auto module=GetModuleHandleA("copvr_bridge.dll");
            if(module)marker_=reinterpret_cast<Marker>(GetProcAddress(module,"copvr_profile_stage"));
        }
        start_=profileClockMs();
        if(marker_)marker_(name_.c_str(),1);
    }
    ~RendererProfilePass(){
        if(!trace_.enabled())return;
        if(marker_)marker_(name_.c_str(),0);
        trace_.record(name_.c_str(),frame_,start_,profileClockMs()-start_);
    }
};
}
