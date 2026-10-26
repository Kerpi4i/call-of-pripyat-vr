#pragma once
#include <windows.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <string>
#include <mutex>
namespace copvr {
inline double profileClockMs() {
    LARGE_INTEGER t,f; QueryPerformanceCounter(&t); QueryPerformanceFrequency(&f);
    return double(t.QuadPart)*1000.0/double(f.QuadPart);
}
class ProfileTrace {
    FILE* file_{};
    bool enabled_{};
    unsigned rows_{};
    std::mutex mutex_;
public:
    explicit ProfileTrace(const char* component) {
        enabled_=std::strstr(GetCommandLineA(),"-copvr_profile")!=nullptr;
        if(!enabled_)return;
        const char* dir=std::getenv("COPVR_PROFILE_DIR");
        const std::string path=std::string(dir?dir:".")+"/"+component+".csv";
        file_=std::fopen(path.c_str(),"wb");
        if(file_)std::fprintf(file_,"frame,stage,start_qpc_ms,duration_ms\n");
        else OutputDebugStringA("CoPVR: cannot open profiling CSV\n");
    }
    ~ProfileTrace(){if(file_)std::fclose(file_);}
    bool enabled() const{return enabled_;}
    void record(const char* stage,uint64_t frame,double start,double duration) {
        if(!file_)return;
        const std::lock_guard<std::mutex> lock(mutex_);
        std::fprintf(file_,"%llu,%s,%.6f,%.6f\n",static_cast<unsigned long long>(frame),stage,start,duration);
        if(++rows_%120==0)std::fflush(file_);
    }
};
class ProfileSpan {
    ProfileTrace& trace_; const char* stage_; uint64_t frame_; double start_{};
public:
    ProfileSpan(ProfileTrace& trace,const char* stage,uint64_t frame):trace_(trace),stage_(stage),frame_(frame) {
        if(trace_.enabled())start_=profileClockMs();
    }
    ~ProfileSpan(){if(trace_.enabled())trace_.record(stage_,frame_,start_,profileClockMs()-start_);}
};
}
