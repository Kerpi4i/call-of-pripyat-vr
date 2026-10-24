#pragma once
#include "vr_bridge_api.h"
#include "body_tracking.hpp"
#include "physical_posture.hpp"
namespace copvr_game {
inline bool standingMode() {
    static bool enabled=[] {
        string_path path; FS.update_path(path,"$game_config$","copvr_equipment.ltx");
        if(!FS.exist(path)) return true;
        CInifile config(path);
        return !config.line_exist("body","standing_mode")||config.r_bool("body","standing_mode");
    }();
    return enabled;
}
inline bool postureHead(CopVrHead& head) {
    head={}; head.abi=COPVR_HEAD_ABI;
    auto library=GetModuleHandleA("copvr_bridge.dll");
    auto get=library?reinterpret_cast<CopVrGetHead>(GetProcAddress(library,"copvr_get_head")):nullptr;
    return get&&get(&head)>0&&head.active;
}
inline bool physicalCrouched() {
    static copvr::PhysicalPostureGate gate;CopVrHead head{};
    const bool valid=standingMode()&&postureHead(head);
    return gate.update(valid,valid?head.position[1]:0.f);
}
inline bool standingHeight(CopVrHeight& value) {
    value={};value.abi=COPVR_HEIGHT_ABI;auto library=GetModuleHandleA("copvr_bridge.dll");
    auto get=library?reinterpret_cast<CopVrGetHeight>(GetProcAddress(library,"copvr_get_height")):nullptr;
    return get&&get(&value)>0;
}
struct HitPosture { const void* owner=nullptr; float scale=1; u32 timestamp=0; };
inline HitPosture& hitPosture() { static HitPosture value; return value; }
inline bool calibrateStandingHeight() {
    CopVrHead head{};if(!postureHead(head)||!copvr::canCalibrateStanding(head.position[1])||physicalCrouched())return false;
    auto library=GetModuleHandleA("copvr_bridge.dll");
    auto calibrate=library?reinterpret_cast<CopVrCalibrateHeight>(GetProcAddress(library,"copvr_calibrate_height")):nullptr;
    return calibrate&&calibrate()>0;
}
}
