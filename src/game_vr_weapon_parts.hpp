#pragma once
#include "game_vr_controls.hpp"
#include "game_vr_magazine.hpp"
#include "game_vr_internal_reload.hpp"
#include "xrCore/Animation/Bone.hpp"

namespace copvr_game {
struct WeaponHandleState {
    u16 weapon=u16(-1),bone=BI_NONE;
    bool armed=false,grabbing=false,pulled=false;
    int side=0;
    Fvector start{},contact{},axis{};
    Fmatrix rest;
    float amount=0,travel=.075f;
};
inline WeaponHandleState& weaponHandleState() { static WeaponHandleState value; return value; }
inline void resetWeaponHandle() { weaponHandleState()=WeaponHandleState{};manipulatingWeapon()=false;partGripAnchor().active=false; }
inline shared_str handleProfile(CWeapon* weapon) {
    if(!weapon)return shared_str("");string256 section;xr_strcpy(section,"handle_vr_");xr_strcat(section,weapon->HudSection().c_str());
    auto ini=equipmentConfig();return ini&&ini->section_exist(section)?shared_str(section):shared_str("");
}
inline bool handleContact(CWeapon* weapon,attachable_hud_item* hud,u16& bone,Fvector& contact,Fvector& axis,float& travel) {
    // Pump shotguns are cycled exclusively through their articulated support grip.
    // Suppress this contact for both grabbing and debug-zone rendering.
    if(auto gun=weapon?weapon->cast_weapon_magazined():nullptr;gun&&gun->VRInternalKind()==1)return false;
    auto ini=equipmentConfig();const auto profile=handleProfile(weapon);
    if(!ini||!profile.size()||!hud||!hud->m_model||hud->m_parent_hud_item!=weapon)return false;
    bone=hud->m_model->LL_BoneID(ini->r_string(profile,"bone"));if(bone==BI_NONE)return false;
    xr_vector<Fmatrix> bind;hud->m_model->LL_GetBindTransform(bind);Fmatrix inverse,delta;
    inverse.invert(bind[bone]);delta.mul_43(hud->m_model->LL_GetTransform(bone),inverse);
    delta.transform_tiny(contact,ini->r_fvector3(profile,"contact"));
    delta.transform_dir(axis,ini->r_fvector3(profile,"axis"));axis.normalize_safe();travel=ini->r_float(profile,"travel");
    if(pSettings->line_exist(weapon->cNameSect(),"copvr_bolt_travel"))travel=pSettings->r_float(weapon->cNameSect(),"copvr_bolt_travel");
    travel=_max(.015f,_min(.18f,travel));return true;
}
inline void applyWeaponHandle(attachable_hud_item* hud) {
    partGripAnchor().active=false;
    if(hud)if(auto gun=smart_cast<CWeaponMagazined*>(hud->m_parent_hud_item)) {
        auto& input=internalInteraction();Fvector contact;
        if(input.held&&input.weapon==gun->ID()&&internalBreakContact(*gun,hud,contact)) {
            hud->m_item_transform.transform_tiny(partGripAnchor().palm,contact);
            partGripAnchor().active=true;partGripAnchor().side=secondaryHand();return;
        }
    }
    auto& state=weaponHandleState();auto weapon=hud?smart_cast<CWeapon*>(hud->m_parent_hud_item):nullptr;
    if(!state.grabbing||!weapon||weapon->ID()!=state.weapon||!hud->m_model||state.bone>=hud->m_model->LL_BoneCount())return;
    auto model=hud->m_model;auto& bone=model->LL_GetBoneInstance(state.bone);
    Fmatrix target=state.rest;target.c.mad(state.axis,state.amount);
    const auto callback=bone.callback();auto param=bone.callback_param();const auto type=bone.callback_type();const auto overwrite=bone.callback_overwrite();
    bone.set_callback(type,[](CBoneInstance* b){b->mTransform=*static_cast<Fmatrix*>(b->callback_param());},&target,TRUE);
    model->CalculateBones_Invalidate();model->CalculateBones(TRUE);
    bone.set_callback(type,callback,param,overwrite);
    Fvector contact=state.contact;contact.mad(state.axis,state.amount);
    hud->m_item_transform.transform_tiny(partGripAnchor().palm,contact);
    partGripAnchor().active=true;partGripAnchor().side=state.side;
}
inline void updateWeaponHandle(CActor& actor,const CopVrControls& controls,const Fmatrix& camera,bool allowed,bool secondaryActive) {
    auto& state=weaponHandleState(); const int side=secondaryHand();
    auto weapon=smart_cast<CWeapon*>(actor.inventory().ActiveItem());
    auto hud=g_player_hud?g_player_hud->attached_item(0):nullptr;
    Fmatrix aim;Fvector grip;
    if(launcherProfile(weapon).size()){resetWeaponHandle();return;}
    if(auto gun=weapon?weapon->cast_weapon_magazined():nullptr) {
        if(gun->VRInternalKind()==1||gun->VRInternalKind()==2||gun->VRInternalKind()==3||gun->VRInternalKind()==5){resetWeaponHandle();return;}
    }
    if(!allowed||secondaryActive||occupiedHands()[side]||!weapon||!hud||hud->m_parent_hud_item!=weapon||
        !hud->m_model||weapon->IsPending()||!hand(controls,side,camera,aim,grip)) { resetWeaponHandle();return; }
    if(state.weapon!=weapon->ID()) { resetWeaponHandle();state.weapon=weapon->ID(); }
    grip=trackedPalm(aim,grip,side);
    const float squeeze=controls.hands[side].squeeze;const bool released=squeeze<.25f;
    if(!state.grabbing&&released)state.armed=true;
    Fmatrix inverse;inverse.invert(hud->m_item_transform);Fvector local;inverse.transform_tiny(local,grip);
    if(state.grabbing) {
        claimedHands()[side]=true;manipulatingWeapon()=true;
        Fvector delta;delta.sub(local,state.start);
        if(delta.square_magnitude()>.30f*.30f||side!=state.side) { resetWeaponHandle();return; }
        state.amount=_max(0.f,_min(state.travel,delta.dotproduct(state.axis)));
        if(!state.pulled&&state.amount>=state.travel*.90f) { state.pulled=true;pulse(side,.35f,30); }
        if(released) {
            const bool cycle=state.pulled;resetWeaponHandle();
            if(cycle) {
                actor.inventory().Action(kWPN_FIRE,CMD_STOP);
                if(auto gun=weapon->cast_weapon_magazined();gun&&gun->VRPhysicalMagazine()) {
                    if(gun->VRRack())pulse(side,.45f,40);
                } else if(auto gun=weapon->cast_weapon_magazined()) {
                    if(gun->VRInternalKind()==1||gun->VRInternalKind()==4){
                        auto ini=equipmentConfig();const auto profile=gun->VRInternalProfile();
                        const bool latch=profile.size()&&ini->line_exist(profile,"load_requires_open")&&ini->r_bool(profile,"load_requires_open");
                        if(latch&&gun->VRInternalState().pump>=.9f)gun->VRInternalPump(0);
                        else {gun->VRInternalPump(1);ejectInternalShells(*gun);if(!latch)gun->VRInternalPump(0);}
                    }
                    else gun->VRManualReload();
                }
                else actor.inventory().Action(kWPN_RELOAD,CMD_START);
                Msg("* CoPVR: physical charging handle cycle weapon=%u",weapon->ID());
            }
        }
        return;
    }
    if(!state.armed||squeeze<.7f)return;
    state.armed=false;
    u16 bone;Fvector contact,axis;float travel;
    if(!handleContact(weapon,hud,bone,contact,axis,travel)||local.distance_to(contact)>.065f)return;
    state.start=local;state.contact=contact;state.axis=axis;state.bone=bone;state.travel=travel;
    state.rest=hud->m_model->LL_GetTransform(bone);state.side=side;state.amount=0;state.grabbing=true;state.pulled=false;
    claimedHands()[side]=true;manipulatingWeapon()=true;
    actor.inventory().Action(kWPN_FIRE,CMD_STOP);resetSupportGrip();pulse(side,.3f,25);
    Msg("* CoPVR: physical charging handle grabbed weapon=%u bone=%s hand=%d",weapon->ID(),hud->m_model->LL_BoneName_dbg(bone),side);
}
}
