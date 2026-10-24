#pragma once
#include "game_vr_controls.hpp"
#include "game_vr_motion.hpp"
#include "xrGame/Actor.h"
#include "xrGame/Inventory.h"
#include "xrGame/WeaponMagazined.h"
#include "xrGame/player_hud.h"
#include "xrGame/Level.h"
#include "Include/xrRender/Kinematics.h"

namespace copvr_game {
inline shared_str magazineProfile(CWeapon* weapon) {
    auto gun=weapon?weapon->cast_weapon_magazined():nullptr;
    return gun?gun->VRMagazineProfile():shared_str("");
}
inline bool magazineFrame(CWeapon* weapon,Fmatrix& delta,Fvector& local,Fvector& world) {
    const auto profile=magazineProfile(weapon); auto ini=equipmentConfig();
    auto hud=g_player_hud?g_player_hud->attached_item(0):nullptr;
    if(!profile.size()||!ini||!hud||hud->m_parent_hud_item!=weapon||!hud->m_model) return false;
    auto model=hud->m_model;const u16 bone=model->LL_BoneID(ini->r_string(profile,"bone"));
    if(bone==BI_NONE) return false;
    // Use the magazine's parent: its transform remains valid with magazine hidden.
    const u16 parent=model->GetBoneData(bone).GetParentID();
    delta.identity();
    if(parent!=BI_NONE) {
        xr_vector<Fmatrix> bind;model->LL_GetBindTransform(bind);Fmatrix inverse;inverse.invert(bind[parent]);
        delta.mul_43(model->LL_GetTransform(parent),inverse);
    }
    delta.transform_tiny(local,ini->r_fvector3(profile,"center"));
    hud->m_item_transform.transform_tiny(world,local); return true;
}
inline bool magazinePort(CWeapon* weapon,Fvector& local,Fvector& world) {
    Fmatrix delta;if(!magazineFrame(weapon,delta,local,world))return false;
    const auto profile=magazineProfile(weapon);auto ini=equipmentConfig();
    if(ini->line_exist(profile,"socket_center")) {
        delta.transform_tiny(local,ini->r_fvector3(profile,"socket_center"));
        g_player_hud->attached_item(0)->m_item_transform.transform_tiny(world,local);
    }
    return true;
}
inline void enforceMagazineVisibility(attachable_hud_item* hud) {
    auto gun=hud?smart_cast<CWeaponMagazined*>(hud->m_parent_hud_item):nullptr;
    if(!gun||!hud->m_model) return;
    const auto profile=magazineProfile(gun);auto ini=equipmentConfig();if(!profile.size()||!ini)return;
    const u16 bone=hud->m_model->LL_BoneID(ini->r_string(profile,"bone"));if(bone==BI_NONE)return;
    // Reapply after animation evaluation, including mixed-weight pistol meshes.
    if(gun->VRMagazinePresent()) {
        if(!hud->m_model->LL_GetBoneVisible(bone)) hud->m_model->LL_SetBoneVisible(bone,TRUE,TRUE);
    } else {
        hud->m_model->LL_SetBoneVisible(bone,FALSE,TRUE);
        hud->m_model->CalculateBones_Invalidate();hud->m_model->CalculateBones(TRUE);
        auto& b=hud->m_model->LL_GetBoneInstance(bone);b.mTransform.scale(0,0,0);b.mRenderTransform.scale(0,0,0);
    }
}
struct DetachedMagazine {
    bool held=false,armed=false;
    shared_str profile;
    Fmatrix pose,offset;
    MotionHistory motion;
};
inline DetachedMagazine* detachedMagazines() { static DetachedMagazine value[2];return value; }
struct DiscardedMagazine {
    shared_str profile; Fmatrix pose; Fvector velocity{}; u32 until=0;
};
inline DiscardedMagazine* discardedMagazines() { static DiscardedMagazine value[24];return value; }
inline void discardMagazine(int side,bool throwing) {
    auto& held=detachedMagazines()[side];
    if(!held.held) return;
    auto objects=discardedMagazines(); int index=0;
    for(int i=0;i<24;++i) if(objects[i].until<objects[index].until) index=i;
    auto& object=objects[index];object.profile=held.profile;object.pose=held.pose;
    object.velocity=throwing?held.motion.throwLinear():Fvector().set(0,0,0);
    object.until=Device.dwTimeGlobal+30000;
    held=DetachedMagazine{}; occupiedHands()[side]=false;heldObjectRadius()[side]=0;
}
inline void resetDetachedMagazines() {
    for(int side=0;side<2;++side) {
        if(detachedMagazines()[side].held) { occupiedHands()[side]=false;heldObjectRadius()[side]=0; }
        detachedMagazines()[side]=DetachedMagazine{};
    }
    for(int i=0;i<24;++i) discardedMagazines()[i].until=0;
}
inline void updateDetachedMagazines(CActor& actor,const CopVrControls& controls,const Fmatrix& camera,bool allowed,bool secondaryActive) {
    static CActor* owner=nullptr;static u32 previousTime=0;
    if(owner!=&actor||Device.dwTimeGlobal<previousTime) {
        for(int i=0;i<2;++i) { if(detachedMagazines()[i].held) occupiedHands()[i]=false;detachedMagazines()[i]=DetachedMagazine{}; }
        for(int i=0;i<24;++i) discardedMagazines()[i].until=0;
        owner=&actor;
    }
    previousTime=Device.dwTimeGlobal;
    const float dt=_min(.05f,Device.fTimeDelta);
    for(int i=0;i<24;++i) {
        auto& object=discardedMagazines()[i];
        if(!object.until||object.until<=Device.dwTimeGlobal) { object.until=0;continue; }
        if(!allowed) continue;
        object.velocity.y-=9.81f*dt; Fvector delta;delta.mul(object.velocity,dt);float distance=delta.magnitude();
        if(distance>.00001f) {
            Fvector direction=delta;direction.div(distance);collide::rq_result hit;
            if(Level().ObjectSpace.RayPick(object.pose.c,direction,distance+.015f,collide::rqtStatic,hit,&actor)) {
                object.pose.c.mad(direction,_max(0.f,hit.range-.015f));object.velocity.set(0,0,0);
            } else object.pose.c.add(delta);
        }
    }
    auto weapon=smart_cast<CWeaponMagazined*>(actor.inventory().ActiveItem());
    auto hud=g_player_hud?g_player_hud->attached_item(0):nullptr;
    for(int side=0;side<2;++side) {
        auto& held=detachedMagazines()[side];Fmatrix pose;Fvector grip;
        const bool tracked=allowed&&hand(controls,side,camera,pose,grip);
        if(held.held) {
            if(!tracked) { discardMagazine(side,false);continue; }
            pose.c=trackedPalm(pose,grip,side);
            held.pose.mul_43(pose,held.offset);
            auto raw=unsmoothedHandPose(pose,camera,side);held.motion.update(raw,Device.fTimeDelta);
            claimedHands()[side]=occupiedHands()[side]=true;heldObjectRadius()[side]=.045f;
            if(controls.hands[side].squeeze<.25f) { discardMagazine(side,true);pulse(side,.25f,25); }
            continue;
        }
        if(!tracked) { held.armed=false;continue; }
        if(controls.hands[side].squeeze<.25f) held.armed=true;
        const bool pressed=held.armed&&controls.hands[side].squeeze>.7f;
        if(pressed) held.armed=false;
        if(!pressed||side!=secondaryHand()||secondaryActive||occupiedHands()[side]||claimedHands()[side]||
            !weapon||!weapon->VRPhysicalMagazine()||!weapon->VRMagazinePresent()||!hud||hud->m_parent_hud_item!=weapon) continue;
        Fvector local,port,visualCenter;Fmatrix magazineDelta;if(!magazineFrame(weapon,magazineDelta,local,visualCenter)||!magazinePort(weapon,local,port)) continue;
        pose.c=trackedPalm(pose,grip,side);
        if(pose.c.distance_to(port)>.065f) continue;
        if(!weapon->VRRemoveMagazine()) continue;
        held.held=true;held.profile=weapon->VRMagazineProfile();
        held.pose.mul_43(hud->m_item_transform,magazineDelta);held.pose.c=visualCenter;
        Fmatrix inverse;inverse.invert(pose);held.offset.mul_43(inverse,held.pose);
        held.motion.reset();claimedHands()[side]=occupiedHands()[side]=true;heldObjectRadius()[side]=.045f;
        resetSupportGrip();actor.inventory().Action(kWPN_FIRE,CMD_STOP);pulse(side,.45f,45);
        Msg("* CoPVR: magazine detached weapon=%u hand=%d",weapon->ID(),side);
    }
}
}
