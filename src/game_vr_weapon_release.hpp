#pragma once
#include "game_vr_world_items.hpp"
#include "Include/xrRender/Kinematics.h"

namespace copvr_game {
struct WeaponReleaseState {
    u16 id=u16(-1); int side=-1; bool held=false; u32 heldSince=0;
    MotionHistory motion;
};
inline WeaponReleaseState& weaponReleaseState() { static WeaponReleaseState value; return value; }
inline void resetWeaponRelease() { weaponReleaseState()=WeaponReleaseState{}; }
inline bool updateWeaponRelease(CActor& actor,const CopVrControls& controls,const Fmatrix& camera,bool allowed) {
    auto& state=weaponReleaseState(); auto weapon=smart_cast<CWeapon*>(actor.inventory().ActiveItem());
    const int side=primaryHand(); const u16 slot=actor.inventory().GetActiveSlot();
    auto hud=g_player_hud?g_player_hud->attached_item(0):nullptr;
    Fmatrix pose; Fvector grip;
    if(!allowed||!weapon||(slot!=INV_SLOT_2&&slot!=INV_SLOT_3)||ownedReleaseFor(weapon->ID())||
        actor.inventory().GetNextActiveSlot()!=slot||!hud||hud->m_parent_hud_item!=weapon||
        !hand(controls,side,camera,pose,grip)) { resetWeaponRelease(); return false; }
    if(state.id!=weapon->ID()||state.side!=side) { resetWeaponRelease(); state.id=weapon->ID(); state.side=side; }
    pose.c=grip; state.motion.update(unsmoothedHandPose(pose,camera,side),Device.fTimeDelta);
    const float squeeze=controls.hands[side].squeeze;
    if(squeeze>.7f&&!state.held) { state.held=true; state.heldSince=Device.dwTimeGlobal; }
    if(!state.held||squeeze>=.25f) return false;
    if(weapon->IsPending()) return false;
    state.held=false;
    if(supportGrip().active()) return false;
    Fvector center,localCenter,desiredCenter,offset;
    Fmatrix inverse; inverse.invert(weapon->XFORM()); weapon->Center(center); inverse.transform_tiny(localCenter,center);
    Fmatrix release=hud->m_item_transform;
    if(hud->m_model) {
        hud->m_model->dcast_RenderVisual()->getVisData().box.getcenter(center);
        release.transform_tiny(desiredCenter,center);
    }
    else desiredCenter=grip;
    release.transform_dir(offset,localCenter); release.c.sub(desiredCenter,offset);
    Fvector angularContribution,lever,velocity=state.motion.throwLinear();
    lever.sub(desiredCenter,grip); angularContribution.crossproduct(state.motion.angular,lever); velocity.add(angularContribution);
    const float speed=velocity.magnitude(); if(speed>12.f) velocity.mul(12.f/speed);
    if(!queueOwnedRelease(actor,*weapon,release,velocity,state.motion.angular)) return false;
    pulse(side,.2f,25); resetWeaponRelease(); return true;
}
}
