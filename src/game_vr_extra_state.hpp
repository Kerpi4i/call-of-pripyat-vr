#pragma once
#include "trigger_gate.hpp"
#include "game_vr_motion.hpp"
class CWeapon;
class CMissile;
namespace copvr_game {
struct ExtraWeapon {
    CWeapon* weapon=nullptr;
    CMissile* tool=nullptr;
    int side=-1; u16 owner=u16(-1);
    bool poseValid=false,held=false;
    Fmatrix pose;
    copvr::TriggerGate trigger;
    MotionHistory motion;
};
inline ExtraWeapon& extraWeapon() { static ExtraWeapon value; return value; }
inline bool isExtraWeapon(const void* weapon) { return weapon&&extraWeapon().weapon==weapon; }
inline bool isExtraTool(const void* tool) { return tool&&extraWeapon().tool==tool; }
inline bool extraHeld() { return extraWeapon().weapon||extraWeapon().tool; }
inline bool extraHand(int side) { return extraHeld()&&extraWeapon().side==side; }
inline void clearExtraWeapon() { extraWeapon()=ExtraWeapon{}; }
}
