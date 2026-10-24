#pragma once
#define XR_NO_PROTOTYPES
#include <openxr/openxr.h>
#include "interaction.hpp"

namespace copvr {
// The renderer owns instance, session, reference space and xrWaitFrame/xrEndFrame.
// Call sample at predictedDisplayTime, only while the session is running.
class OpenXRInput {
    PFN_xrGetInstanceProcAddr get_{};
    XrInstance instance_{}; XrSession session_{}; XrSpace reference_{};
    XrActionSet actions_{};
    XrAction grip_{},aim_{},squeeze_{},trigger_{},haptic_{};
    XrAction stick_{},primary_{},secondary_{},stickClick_{},menu_{};
    std::array<XrPath,2> paths_{};
    std::array<XrSpace,2> grips_{},aims_{};
    XrSpace head_{},floor_{};
    template<class T> T proc(const char* name) const {
        PFN_xrVoidFunction fn{};
        check(get_(instance_,name,&fn),name);
        return reinterpret_cast<T>(fn);
    }
    static void check(XrResult,const char*);
    XrPath path(const char*) const;
    XrAction action(const char*,XrActionType);
    Pose locate(XrSpace,XrTime,Vec3* =nullptr,bool* =nullptr) const;
public:
    OpenXRInput(PFN_xrGetInstanceProcAddr,XrInstance,XrSession,XrSpace);
    ~OpenXRInput();
    OpenXRInput(const OpenXRInput&)=delete;
    OpenXRInput& operator=(const OpenXRInput&)=delete;
    Frame sample(XrTime predictedDisplayTime,bool focused);
    void pulse(int hand,float amplitude,uint32_t milliseconds);
};
}
