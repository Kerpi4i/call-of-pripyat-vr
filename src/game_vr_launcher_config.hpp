#pragma once
#include "game_vr_controls.hpp"
#include "xrGame/WeaponMagazinedWGrenade.h"
#include "xrGame/player_hud.h"
namespace copvr_game {
inline shared_str launcherProfile(CWeapon* weapon) {
    auto gl=weapon?smart_cast<CWeaponMagazinedWGrenade*>(weapon):nullptr;
    if(!gl||!gl->IsGrenadeLauncherAttached()||!gl->m_bGrenadeMode)return shared_str("");
    string256 name;xr_strcpy(name,"launcher_vr_");xr_strcat(name,gl->HudSection().c_str());
    auto ini=equipmentConfig();return ini&&ini->section_exist(name)?shared_str(name):shared_str("");
}
inline bool launcherPort(CWeapon* weapon,Fvector& local,Fvector& world) {
    const auto profile=launcherProfile(weapon);auto hud=g_player_hud?g_player_hud->attached_item(0):nullptr;
    if(!profile.size()||!hud||hud->m_parent_hud_item!=weapon||!hud->m_model)return false;
    auto ini=equipmentConfig();const auto bone=hud->m_model->LL_BoneID(ini->r_string(profile,"bone"));if(bone==BI_NONE)return false;
    hud->m_model->LL_GetTransform(bone).transform_tiny(local,ini->r_fvector3(profile,"port"));
    hud->m_item_transform.transform_tiny(world,local);return true;
}
}
