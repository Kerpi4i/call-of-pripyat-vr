#pragma once
#include "game_vr_extra_state.hpp"
namespace copvr_game {
// Independent second-hand equipment is disabled until its ownership lifecycle
// can be made reliable. All equips use the native inventory/HUD path.
inline bool equipExtraWeapon(CActor&,CWeapon&,int) { return false; }
inline bool equipExtraTool(CActor&,CMissile&,int) { return false; }
inline void updateExtraWeapon(CActor&,const CopVrControls&,const Fmatrix&,bool) {}
}
