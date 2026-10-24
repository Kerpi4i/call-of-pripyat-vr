#pragma once
#include "game_vr_controls.hpp"
#include "xrGame/Weapon.h"
#include "xrGame/Actor.h"
#include "xrGame/player_hud.h"
namespace copvr_game {
struct ProjectileWeaponPose { const CWeapon* weapon=nullptr;Fmatrix pose;u32 frame=0;bool valid=false; };
inline ProjectileWeaponPose& projectileWeaponPose(){static ProjectileWeaponPose value;return value;}
inline void rememberProjectileWeaponPose(attachable_hud_item* hud) {
    auto& stored=projectileWeaponPose();auto weapon=hud?smart_cast<CWeapon*>(hud->m_parent_hud_item):nullptr;
    stored.valid=weapon&&_valid(hud->m_item_transform.c)&&_valid(hud->m_item_transform.k);
    stored.weapon=weapon;stored.frame=Device.dwFrame;if(stored.valid)stored.pose=hud->m_item_transform;
}
inline bool trackedProjectileMuzzle(CWeapon& weapon,bool secondary,Fvector& position,Fvector& direction) {
    if(!strstr(Core.Params,"-copvr")||!smart_cast<CActor*>(weapon.H_Parent()))return false;
    auto hud=weapon.HudItemData();const auto& stored=projectileWeaponPose();
    if(!hud||!hud->m_model||!stored.valid||stored.weapon!=&weapon||Device.dwFrame-stored.frame>2)return false;
    const auto& measures=hud->m_measures;
    const auto flag=secondary?hud_item_measures::e_fire_point2:hud_item_measures::e_fire_point;
    if(!measures.m_prop_flags.test(flag))return false;
    const auto bone=secondary?measures.m_fire_bone2:measures.m_fire_bone;
    if(bone==BI_NONE||bone>=hud->m_model->LL_BoneCount())return false;
    Fvector local;hud->m_model->LL_GetTransform(bone).transform_tiny(local,secondary?measures.m_fire_point2_offset:measures.m_fire_point_offset);
    stored.pose.transform_tiny(position,local);direction=stored.pose.k;direction.normalize_safe();
    return _valid(position)&&_valid(direction)&&direction.square_magnitude()>.9f;
}
}
