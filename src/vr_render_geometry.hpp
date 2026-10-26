#pragma once
#include <cmath>
#include <cstdint>
#include <cstdio>
namespace copvr {
// Legacy shadow warping expects a centered perspective frustum. Enclose the
// off-axis eye rays in a padded symmetric frustum instead of copying offsets.
template<class Matrix> bool conservativeShadowProjection(Matrix& result,const Matrix& eye,float nearPlane,float farPlane) {
    if(!std::isfinite(eye._11)||!std::isfinite(eye._22)||!std::isfinite(eye._31)||!std::isfinite(eye._32)||
        eye._11<=0||eye._22<=0||!std::isfinite(nearPlane)||!std::isfinite(farPlane)||nearPlane<=0||farPlane<=nearPlane)return false;
    const float halfX=(1.f+std::abs(eye._31))/eye._11*1.08f;
    const float halfY=(1.f+std::abs(eye._32))/eye._22*1.08f;
    if(!std::isfinite(halfX)||!std::isfinite(halfY))return false;
    result=eye;result._11=1.f/halfX;result._22=1.f/halfY;
    result._31=result._32=0.f;
    result._33=farPlane/(farPlane-nearPlane);result._43=-nearPlane*result._33;
    return true;
}
struct RenderSize { uint32_t width=0,height=0; };
inline RenderSize parseEyeSize(const char* text) {
    unsigned w=0,h=0;char extra=0;
    if(!text||std::sscanf(text,"%ux%u%c",&w,&h,&extra)<2||
        (extra&&extra!=' '&&extra!='\t')||w<512||h<512||w>4096||h>4096)return {};
    return {w,h};
}
// Keep off-axis eye rays intact while fitting a legacy shadow depth interval.
template<class Matrix> bool projectionWithFar(Matrix& result,const Matrix& eye,float farPlane) {
    if(!std::isfinite(eye._33)||!std::isfinite(eye._43)||eye._33==0.f)return false;
    const float nearPlane=-eye._43/eye._33;
    if(!std::isfinite(farPlane)||nearPlane<=0||farPlane<=nearPlane)return false;
    result=eye;
    result._33=farPlane/(farPlane-nearPlane);
    result._43=-nearPlane*result._33;
    return true;
}
}
