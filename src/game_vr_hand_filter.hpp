#pragma once
#include <algorithm>
#include "adaptive_pose_filter.hpp"
namespace copvr_game {
struct HandFilterState { copvr::AdaptivePoseFilter filter; Fmatrix rawGrip,filteredGrip; bool ready=false; };
inline HandFilterState* handFilters() { static HandFilterState value[2]; return value; }
inline bool handSmoothingEnabled() {
    auto ini=equipmentConfig(); return !ini||!ini->line_exist("hands","smoothing_enabled")||ini->r_bool("hands","smoothing_enabled");
}
inline copvr::AdaptivePoseSettings handFilterSettings() {
    static const auto settings=[] {
        copvr::AdaptivePoseSettings s; auto ini=equipmentConfig(); if(!ini) return s;
        auto read=[&](const char* name,float value,float low,float high) { return ini->line_exist("hands",name)?std::clamp(ini->r_float("hands",name),low,high):value; };
        s.positionCutoff=read("position_min_cutoff",s.positionCutoff,1,120);
        s.rotationCutoff=read("rotation_min_cutoff",s.rotationCutoff,1,120);
        s.positionBeta=read("position_smoothing_beta",s.positionBeta,0,100);
        s.rotationBeta=read("rotation_smoothing_beta",s.rotationBeta,0,100);
        return s;
    }(); return settings;
}
inline Fmatrix smoothHandGrip(int side,const Fmatrix& raw,uint64_t timestamp) {
    auto& state=handFilters()[side]; state.rawGrip=raw; state.ready=true;
    if(!handSmoothingEnabled()) { state.filter.reset(); state.filteredGrip=raw; return raw; }
    const auto q=copvr::basisQuaternion({raw.i.x,raw.i.y,raw.i.z},{raw.j.x,raw.j.y,raw.j.z},{raw.k.x,raw.k.y,raw.k.z});
    const auto pose=state.filter.update({{raw.c.x,raw.c.y,raw.c.z},q,true},timestamp,handFilterSettings());
    const auto i=copvr::rotate(pose.orientation,{1,0,0}),j=copvr::rotate(pose.orientation,{0,1,0}),k=copvr::rotate(pose.orientation,{0,0,1});
    Fmatrix filtered; filtered.set(Fvector().set(i.x,i.y,i.z),Fvector().set(j.x,j.y,j.z),Fvector().set(k.x,k.y,k.z),Fvector().set(pose.position.x,pose.position.y,pose.position.z));
    state.filteredGrip=filtered; return filtered;
}
inline Fmatrix unsmoothedHandPose(const Fmatrix& pose,const Fmatrix& camera,int side) {
    const auto& state=handFilters()[side]; if(!state.ready||!handSmoothingEnabled()) return pose;
    Fmatrix raw,filtered,inverse,delta,result; raw.mul_43(camera,state.rawGrip); filtered.mul_43(camera,state.filteredGrip);
    inverse.invert(filtered); delta.mul_43(raw,inverse); result.mul_43(delta,pose); return result;
}
}
