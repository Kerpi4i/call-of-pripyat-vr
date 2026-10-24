#pragma once
#include "game_vr_magazine.hpp"
#include "game_vr_arms.hpp"
#include "game_vr_reload_grip.hpp"
#include "xrGame/WeaponAmmo.h"

namespace copvr_game {
inline Fmatrix internalBodyDelta(attachable_hud_item* hud) {
    Fmatrix delta;delta.identity();if(!hud||!hud->m_model)return delta;
    xr_vector<Fmatrix> bind;hud->m_model->LL_GetBindTransform(bind);
    const u16 body=hud->m_model->LL_BoneID("body");
    const u16 root=body==BI_NONE?hud->m_model->LL_GetBoneRoot():body;Fmatrix inverse;inverse.invert(bind[root]);
    delta.mul_43(hud->m_model->LL_GetTransform(root),inverse);return delta;
}
inline Fmatrix internalPartDelta(CWeaponMagazined& gun,attachable_hud_item* hud) {
    Fmatrix body=internalBodyDelta(hud),part;part.identity();
    auto& state=gun.VRInternalState();const auto profile=gun.VRInternalProfile();auto ini=equipmentConfig();
    if(gun.VRInternalKind()==1)part.c.z=-state.pump*ini->r_float(profile,"travel");
    if(gun.VRInternalKind()==2) {
        const u16 bone=hud->m_model->LL_BoneID(ini->r_string(profile,"hinge"));
        if(bone!=BI_NONE) {
            xr_vector<Fmatrix> bind;hud->m_model->LL_GetBindTransform(bind);
            const Fvector pivot=bind[bone].c;part.rotateX(state.angle);
            Fvector rotated;part.transform_dir(rotated,pivot);part.c.sub(pivot,rotated);
        }
    }
    Fmatrix result;result.mul_43(body,part);return result;
}
struct DrumOpeningPose {
    shared_str visual;bool ready=false;
    Fmatrix closed[2],opened[2];
};
inline DrumOpeningPose& drumOpeningPose() { static DrumOpeningPose pose;return pose; }
inline void applyInternalDrumPose(CWeaponMagazined& gun,attachable_hud_item* hud) {
    auto ini=equipmentConfig();const auto profile=gun.VRInternalProfile();auto model=hud->m_model;
    auto& opening=drumOpeningPose();
    if(opening.visual!=hud->m_visual_name) {
        opening=DrumOpeningPose{};opening.visual=hud->m_visual_name;
        ReloadSampleVisual sample(hud->m_visual_name.c_str());auto animation=smart_cast<IKinematicsAnimated*>(sample.visual);
        if(animation) {
            auto source=animation->dcast_PKinematics();
            auto& root=source->LL_GetBoneInstance(source->LL_GetBoneRoot());root.set_callback_overwrite(TRUE);root.mTransform.identity();
            const char* names[]={"lock","reload"};
            const auto idle=animation->ID_Cycle_Safe("idle"),open=animation->ID_Cycle_Safe(ini->r_string(profile,"open_motion"));
            bool valid=sampleReloadMotion(animation,idle,0.f);
            for(int b=0;b<2&&valid;++b){const auto id=source->LL_BoneID(names[b]);if(id==BI_NONE)valid=false;else opening.closed[b]=source->LL_GetTransform(id);}
            valid=valid&&sampleReloadMotion(animation,open,.98f);
            for(int b=0;b<2&&valid;++b)opening.opened[b]=source->LL_GetTransform(source->LL_BoneID(names[b]));
            opening.ready=valid;Msg("* CoPVR: RG6 authored opening pose valid=%d",valid?1:0);
        }
    }
    auto& state=gun.VRInternalState();const float fraction=_max(0.f,_min(1.f,state.angle/.65f));
    const Fmatrix body=internalBodyDelta(hud);xr_vector<Fmatrix> bind;model->LL_GetBindTransform(bind);
    BoneOverride overrides[3];unsigned count=0;
    if(opening.ready)for(int b=0;b<2;++b) {
        const u16 id=model->LL_BoneID(b==0?"lock":"reload");if(id==BI_NONE)continue;
        Fquaternion a,bq,q;a.set(opening.closed[b]);bq.set(opening.opened[b]);q.slerp(a,bq,fraction);
        Fmatrix local,target;
        if(b==1) {
            // Preserve the authored hinge axis, but extend its travel to 180 degrees.
            Fmatrix inverse,relative;inverse.invert(opening.closed[b]);relative.mul_43(inverse,opening.opened[b]);
            Fquaternion delta;delta.set(relative);if(delta.w<0){delta.w=-delta.w;delta.x=-delta.x;delta.y=-delta.y;delta.z=-delta.z;}
            Fvector axis;float authoredAngle=0;
            if(delta.get_axis_angle(axis,authoredAngle)) {
                axis.normalize_safe();delta.rotation(axis,PI*fraction);Fmatrix rotation;rotation.rotation(delta);local.mul_43(opening.closed[b],rotation);
            } else local.rotation(q);
        } else local.rotation(q);
        local.c.lerp(opening.closed[b].c,opening.opened[b].c,fraction);
        target.mul_43(body,local);overrides[count++].set(model->LL_GetBoneInstance(id),target);
    }
    const u16 drum=model->LL_BoneID(ini->r_string(profile,"drum"));
    if(drum!=BI_NONE) {
        Fmatrix turn,local,target;turn.rotateZ(-state.pump*2.f*PI);
        Fvector shifted;turn.transform_dir(shifted,bind[drum].c);turn.c.sub(bind[drum].c,shifted);
        local.mul_43(turn,bind[drum]);target.mul_43(body,local);overrides[count++].set(model->LL_GetBoneInstance(drum),target);
    }
    const u16 feed=model->LL_BoneID(ini->r_string(profile,"feed_bone"));if(feed!=BI_NONE)model->LL_SetBoneVisible(feed,FALSE,TRUE);
    model->CalculateBones_Invalidate();model->CalculateBones(TRUE);
}
inline bool internalDrumRoundPose(CWeaponMagazined& gun,attachable_hud_item* hud,unsigned chamber,Fmatrix& world) {
    auto ini=equipmentConfig();const auto profile=gun.VRInternalProfile();auto model=hud->m_model;
    const u16 drum=model->LL_BoneID(ini->r_string(profile,"drum"));if(drum==BI_NONE)return false;
    xr_vector<Fmatrix> bind;model->LL_GetBindTransform(bind);Fmatrix inverse,delta,turn,local;
    inverse.invert(bind[drum]);delta.mul_43(model->LL_GetTransform(drum),inverse);
    turn.rotateZ(float(chamber)*PI/3.f);Fvector shifted;turn.transform_dir(shifted,bind[drum].c);turn.c.sub(bind[drum].c,shifted);
    local.mul_43(delta,turn);world.mul_43(hud->m_item_transform,local);
    Fvector center;world.transform_tiny(center,ini->r_fvector3("internal_round_vog25","center"));world.c=center;return true;
}
inline bool internalPort(CWeaponMagazined& gun,int chamber,Fvector& local,Fvector& world) {
    auto hud=g_player_hud?g_player_hud->attached_item(0):nullptr;
    if(!hud||hud->m_parent_hud_item!=&gun||!hud->m_model)return false;
    string32 key;xr_sprintf(key,"port%d",chamber);auto ini=equipmentConfig();const auto profile=gun.VRInternalProfile();
    if(!profile.size()||!ini->line_exist(profile,key))return false;
    Fmatrix delta=gun.VRInternalKind()==2?internalPartDelta(gun,hud):internalBodyDelta(hud);
    delta.transform_tiny(local,ini->r_fvector3(profile,key));hud->m_item_transform.transform_tiny(world,local);return true;
}
inline bool internalRoundAtWeapon(CActor& actor,const Fvector& palm,u16 weaponId,int& chamber) {
    auto gun=smart_cast<CWeaponMagazined*>(actor.inventory().ActiveItem());
    if(!gun||gun->ID()!=weaponId||!gun->VRInternalKind()||gun->IsPending()||gun->GetAmmoElapsed()>=gun->GetAmmoMagSize())return false;
    const int kind=gun->VRInternalKind();
    if(kind==1) {
        auto ini=equipmentConfig();const auto profile=gun->VRInternalProfile();
        if(!gun->VRInternalState().canLoadTube(ini->line_exist(profile,"load_requires_open")&&ini->r_bool(profile,"load_requires_open")))return false;
    }
    if(kind==5&&!gun->VRInternalState().drumCanLoad())return false;float best=kind==3?.10f:.065f;chamber=-1;
    for(int i=0;i<(kind==2?2:1);++i) {
        if(kind==2&&!gun->VRInternalState().canLoad(i))continue;
        Fvector local,world;if(!internalPort(*gun,i,local,world))continue;
        const float distance=palm.distance_to(world);
        if(distance<best){best=distance;chamber=i;interactionZones()[secondaryHand()].reload=true;interactionZones()[secondaryHand()].port=world;}
    }
    return chamber>=0;
}
inline void ejectInternalShells(CWeaponMagazined& gun) {
    const auto mask=gun.VRInternalEject();if(!mask)return;
    auto hud=g_player_hud?g_player_hud->attached_item(0):nullptr;if(!hud||hud->m_parent_hud_item!=&gun)return;
    for(int chamber=0;chamber<2;++chamber)if(mask&(1u<<chamber)) {
        auto objects=discardedMagazines();int index=0;
        for(int i=1;i<24;++i)if(objects[i].until<objects[index].until)index=i;
        auto& shell=objects[index];shell.profile="internal_round_12g";
        Fmatrix localPose=internalPartDelta(gun,hud);shell.pose.mul_43(hud->m_item_transform,localPose);
        Fvector local,world;if(internalPort(gun,gun.VRInternalKind()==2?chamber:0,local,world))shell.pose.c=world;
        Fvector velocity;velocity.set(chamber==0?-.15f:.15f,.65f,-1.8f);
        shell.pose.transform_dir(shell.velocity,velocity);shell.until=Device.dwTimeGlobal+15000;
    }
}
struct InternalSupportContact {
    const CWeapon* weapon=nullptr;const IKinematics* model=nullptr;int primary=-1;
    Fvector foregrip{};Fmatrix left,right;
};
inline InternalSupportContact& internalSupportContact() { static InternalSupportContact value;return value; }
inline bool sampleInternalSupport(attachable_hud_item* hud,InternalSupportContact& contact) {
    if(!g_player_hud)return false;const auto section=g_player_hud->section_name();
    if(!pSettings->line_exist(section,"visual")||!pSettings->line_exist(section,"ancor_0"))return false;
    auto motion=hud->m_hand_motions.find_motion("anm_idle_2");if(!motion)motion=hud->m_hand_motions.find_motion("anm_idle");
    if(!motion||motion->m_animations.empty())return false;
    ReloadSampleVisual handsVisual(pSettings->r_string(section,"visual")),itemVisual(hud->m_visual_name.c_str());
    auto hands=smart_cast<IKinematicsAnimated*>(handsVisual.visual),item=smart_cast<IKinematicsAnimated*>(itemVisual.visual);
    if(!hands||!item)return false;auto model=hands->dcast_PKinematics(),itemModel=item->dcast_PKinematics();
    auto& root=itemModel->LL_GetBoneInstance(itemModel->LL_GetBoneRoot());root.set_callback_overwrite(TRUE);root.mTransform.identity();
    const auto handMotion=hands->ID_Cycle_Safe(motion->m_animations[0].name);
    auto itemMotion=item->ID_Cycle_Safe(motion->m_base_name!=motion->m_additional_name?motion->m_additional_name:motion->m_animations[0].name);
    if(!itemMotion.valid())itemMotion=item->ID_Cycle_Safe("idle");
    if(!sampleReloadMotion(hands,handMotion,0.f)||!sampleReloadMotion(item,itemMotion,0.f))return false;
    const u16 wrist=wristBone(model,0,BI_NONE),anchor=model->LL_BoneID(pSettings->r_string(section,"ancor_0"));
    if(wrist==BI_NONE||anchor==BI_NONE)return false;
    Fmatrix attachment,inverse,local,inverseBody;
    attachment.mul_43(model->LL_GetTransform(anchor),hud->m_attach_offset);inverse.invert(attachment);
    local.mul_43(inverse,model->LL_GetTransform(wrist));
    // Store immutable idle contacts in bind-object space, never in a transient
    // knife/show/reload frame. Runtime articulation is applied exactly once.
    xr_vector<Fmatrix> bind;itemModel->LL_GetBindTransform(bind);const u16 body=itemModel->LL_BoneID("body");
    const u16 base=body==BI_NONE?itemModel->LL_GetBoneRoot():body;
    Fmatrix inverseBind,delta;inverseBind.invert(bind[base]);delta.mul_43(itemModel->LL_GetTransform(base),inverseBind);inverseBody.invert(delta);
    contact.left.mul_43(inverseBody,local);
    if(!oppositeWrist(model,contact.left,0,1,contact.right))return false;
    const int side=secondaryHand();const auto& support=side==0?contact.left:contact.right;
    if(palmSockets()[side].valid)support.transform_tiny(contact.foregrip,palmSockets()[side].boneLocal);else contact.foregrip=support.c;
    return true;
}
// Separate barrel contact; ordinary foregrip never operates a break action.
inline bool internalBreakContact(CWeaponMagazined& gun,attachable_hud_item* hud,Fvector& local) {
    auto& neutral=internalSupportContact();
    if(gun.VRInternalKind()!=2||neutral.weapon!=&gun||neutral.model!=hud->m_model||neutral.primary!=primaryHand())return false;
    Fvector source=neutral.foregrip;source.z+=.30f;
    internalPartDelta(gun,hud).transform_tiny(local,source);return true;
}
inline void applyInternalWeaponPose(attachable_hud_item* hud) {
    auto gun=hud?smart_cast<CWeaponMagazined*>(hud->m_parent_hud_item):nullptr;
    if(!gun||!hud->m_model||!gun->VRInternalKind())return;
    const int kind=gun->VRInternalKind();if(kind==5){applyInternalDrumPose(*gun,hud);return;}if(kind!=1&&kind!=2)return;
    auto ini=equipmentConfig();const auto profile=gun->VRInternalProfile();auto model=hud->m_model;
    auto& state=gun->VRInternalState();xr_vector<Fmatrix> bind;model->LL_GetBindTransform(bind);
    const Fmatrix part=internalPartDelta(*gun,hud);const Fmatrix body=internalBodyDelta(hud);
    BoneOverride overrides[5];unsigned count=0;
    auto overrideBone=[&](const char* name,const Fmatrix& delta) {
        const u16 bone=model->LL_BoneID(name);if(bone==BI_NONE||count>=5)return;
        Fmatrix target;target.mul_43(delta,bind[bone]);overrides[count++].set(model->LL_GetBoneInstance(bone),target);
    };
    if(kind==1) {
        if(ini->line_exist(profile,"feed_bone")) {
            const u16 feed=model->LL_BoneID(ini->r_string(profile,"feed_bone"));
            if(feed!=BI_NONE)model->LL_SetBoneVisible(feed,FALSE,TRUE);
        }
        overrideBone(ini->r_string(profile,"pump_bone"),part);
        if(xr_strcmp(ini->r_string(profile,"pump_bone"),ini->r_string(profile,"bolt_bone"))) {
            Fmatrix shift;shift.identity();shift.c.z=-state.pump*ini->r_float(profile,"travel")*.65f;
            Fmatrix delta;delta.mul_43(body,shift);overrideBone(ini->r_string(profile,"bolt_bone"),delta);
        }
    } else {
        overrideBone(ini->r_string(profile,"hinge"),part);
        for(int i=0;i<2;++i) {
            string32 key;xr_sprintf(key,"chamber%d",i);const char* name=ini->r_string(profile,key);
            const u16 bone=model->LL_BoneID(name);if(bone==BI_NONE)continue;
            const BOOL visible=((state.live|state.spent)&(1u<<i))?TRUE:FALSE;
            if(model->LL_GetBoneVisible(bone)!=visible)model->LL_SetBoneVisible(bone,visible,TRUE);
            if(visible)overrideBone(name,part);
        }
    }
    model->CalculateBones_Invalidate();model->CalculateBones(TRUE);
    auto& contact=gripProfile();auto& articulation=articulatedSupport();
    if(contact.valid&&contact.weapon==gun) {
        auto& neutral=internalSupportContact();
        if(neutral.weapon!=gun||neutral.model!=model||neutral.primary!=primaryHand()) {
            neutral.weapon=gun;neutral.model=model;neutral.primary=primaryHand();
            if(!sampleInternalSupport(hud,neutral)) {
                Fmatrix inverseBody;inverseBody.invert(body);
                inverseBody.transform_tiny(neutral.foregrip,contact.foregrip);
                neutral.left.mul_43(inverseBody,contact.left);neutral.right.mul_43(inverseBody,contact.rightSupport);
                Msg("! CoPVR: using fallback internal support contact weapon=%u",gun->ID());
            }
        }
        const Fmatrix relative=part;
        articulation.weapon=gun;articulation.pointValid=true;relative.transform_tiny(articulation.point,neutral.foregrip);
        if(supportGrip().active()) {
            Fmatrix local,world;local.mul_43(relative,secondaryHand()==0?neutral.left:neutral.right);
            world.mul_43(hud->m_item_transform,local);contact.worldLeft=world;contact.locked=true;
        }
    }
}
struct InternalInteraction {
    const CActor* actor=nullptr;u16 weapon=u16(-1);int primary=-1;bool held=false,closing=false,armed=false,operated=false;
    Fvector start{},anchor{};float initial=0;u32 releasedAt=0;
    MotionHistory motion;
};
inline InternalInteraction& internalInteraction() { static InternalInteraction value;return value; }
inline void resetInternalInteraction() { internalInteraction()=InternalInteraction{};articulatedSupport()=ArticulatedSupport{}; }
inline void lockInternalAim(CWeaponMagazined& gun,const CopVrControls& controls,const Fmatrix& camera,attachable_hud_item* hud) {
    auto& frame=articulatedSupport();if(frame.weapon==&gun&&frame.aimLocked)return;
    Fmatrix aim;Fvector grip;if(!hand(controls,primaryHand(),camera,aim,grip))return;
    Fmatrix inverse;inverse.invert(aim);frame.aimOffset.mul_43(inverse,hud->m_item_transform);frame.aimOffset.c.set(0,0,0);
    frame.weapon=&gun;frame.aimLocked=true;
}
inline void updateInternalInteraction(CActor& actor,const CopVrControls& controls,const Fmatrix& camera,bool allowed,bool secondaryActive) {
    auto& input=internalInteraction();auto gun=smart_cast<CWeaponMagazined*>(actor.inventory().ActiveItem());
    auto hud=g_player_hud?g_player_hud->attached_item(0):nullptr;
    if(!allowed||!gun||!hud||hud->m_parent_hud_item!=gun||!gun->VRInternalKind()) { resetInternalInteraction();return; }
    const int kind=gun->VRInternalKind();if(kind!=1&&kind!=2&&kind!=5){resetInternalInteraction();return;}
    if(input.actor!=&actor||input.weapon!=gun->ID()||input.primary!=primaryHand()) {
        resetInternalInteraction();input.actor=&actor;input.weapon=gun->ID();input.primary=primaryHand();
    }
    auto& state=gun->VRInternalState();const int side=secondaryHand();
    Fmatrix aim;Fvector grip;const bool tracked=hand(controls,side,camera,aim,grip);
    if(tracked)grip=trackedPalm(aim,grip,side);
    Fmatrix inverse;inverse.invert(hud->m_item_transform);Fvector local;if(tracked)inverse.transform_tiny(local,grip);
    const float squeeze=controls.hands[side].squeeze;
    if(squeeze<.25f)input.armed=true;
    if(kind==5) {
        Fvector latch;internalBodyDelta(hud).transform_tiny(latch,equipmentConfig()->r_fvector3(gun->VRInternalProfile(),"latch"));
        const bool available=tracked&&!secondaryActive&&!occupiedHands()[side];
        if(!input.held&&input.armed&&squeeze>=.7f) {
            input.armed=false;
            if(available&&!claimedHands()[side]&&local.distance_to(latch)<.055f) {
                input.held=true;input.operated=false;input.start=local;input.anchor=internalBodyDelta(hud).k;input.anchor.normalize_safe();
                resetSupportGrip();lockInternalAim(*gun,controls,camera,hud);pulse(side,.25f,20);
            }
        }
        if(input.held) {
            if(!available||squeeze<.25f) {input.held=false;articulatedSupport().aimLocked=false;}
            else {
                claimedHands()[side]=true;Fvector delta;delta.sub(local,input.start);
                if(!input.operated&&delta.dotproduct(input.anchor)<=-.04f) {
                    input.operated=true;state.drumToggle();resetSupportGrip();
                    actor.inventory().Action(kWPN_FIRE,CMD_STOP);gun->VRPlayMechanical(state.needsPump?0:2);pulse(side,.45f,45);
                }
            }
        }
        const float dt=_min(.05f,Device.fTimeDelta);
        const float target=state.needsPump?.65f:0.f,step=dt*1.8f;
        state.angle=state.angle<target?_min(target,state.angle+step):_max(target,state.angle-step);
        state.drumAnimate(dt);
        if(state.angle>.03f||state.needsPump)resetSupportGrip();
        return;
    }
    Fvector breakPoint;const bool breakValid=kind==2&&internalBreakContact(*gun,hud,breakPoint);
    const bool available=tracked&&!secondaryActive&&!occupiedHands()[side];
    bool held=available&&squeeze>=.25f&&(kind==1?supportGrip().active():input.held);
    if(kind==2&&!input.held&&available&&input.armed&&squeeze>=.7f) {
        input.armed=false;
        held=breakValid&&local.distance_to(breakPoint)<=.045f;
        if(held){resetSupportGrip();pulse(side,.35f,25);}
    }
    if(kind==2&&held)claimedHands()[side]=true;
    if(held&&!input.held) {
        input.start=local;input.anchor=articulatedSupport().weapon==gun&&articulatedSupport().pointValid?articulatedSupport().point:local;
        if(kind==2&&breakValid)input.anchor=breakPoint;
        input.initial=kind==1?state.pump:state.angle;input.closing=false;
        lockInternalAim(*gun,controls,camera,hud);
    }
    if(!held&&input.held)input.releasedAt=Device.dwTimeGlobal;
    input.held=held;
    if(held) {
        Fvector delta;delta.sub(local,input.start);
        if(kind==1) {
            const float travel=equipmentConfig()->r_float(gun->VRInternalProfile(),"travel");
            Fvector axis=internalBodyDelta(hud).k;axis.normalize_safe();
            const float target=_max(0.f,_min(1.f,input.initial-delta.dotproduct(axis)/travel));
            if(target>.08f||state.pump>.08f)lockInternalAim(*gun,controls,camera,hud);
            const bool wasRear=state.rearReached;
            gun->VRInternalPump(target);
            if(!wasRear&&state.rearReached)pulse(side,.35f,25);
            if(wasRear&&!state.rearReached&&target<=.08f)pulse(side,.3f,20);
            if(state.rearReached&&state.spent)ejectInternalShells(*gun);
        } else {
            xr_vector<Fmatrix> bind;hud->m_model->LL_GetBindTransform(bind);
            const u16 hinge=hud->m_model->LL_BoneID(equipmentConfig()->r_string(gun->VRInternalProfile(),"hinge"));
            Fmatrix inverseBody;inverseBody.invert(internalBodyDelta(hud));Fvector startPoint,currentPoint;
            Fvector anchored;anchored.add(input.anchor,delta);
            inverseBody.transform_tiny(startPoint,input.anchor);inverseBody.transform_tiny(currentPoint,anchored);
            startPoint.sub(bind[hinge].c);currentPoint.sub(bind[hinge].c);
            float difference=atan2f(-currentPoint.y,currentPoint.z)-atan2f(-startPoint.y,startPoint.z);
            if(difference>PI)difference-=2*PI;if(difference<-PI)difference+=2*PI;
            const float target=_max(0.f,_min(.65f,input.initial+difference));
            if(target>.03f||state.angle>.03f)lockInternalAim(*gun,controls,camera,hud);
            const float before=state.angle;gun->VRInternalBreak(target);
            if(before<.5f&&state.angle>=.5f)pulse(side,.3f,25);
            if(before>=.05f&&state.angle<.05f)pulse(side,.35f,25);
            if(state.angle>=.5f&&state.spent)ejectInternalShells(*gun);
        }
    }
    Fmatrix mainPose;Fvector mainGrip;
    if(hand(controls,primaryHand(),camera,mainPose,mainGrip)) {
        mainPose.c=mainGrip;Fmatrix raw;
        if(handFilters()[primaryHand()].ready)raw=handFilters()[primaryHand()].rawGrip;
        else { Fmatrix inverseCamera;inverseCamera.invert(camera);raw.mul_43(inverseCamera,mainPose); }
        input.motion.update(raw,Device.fTimeDelta);
        if(kind==2&&state.angle>.4f&&!held&&!occupiedHands()[side]&&Device.dwTimeGlobal>input.releasedAt+250&&
            input.motion.valid&&(input.motion.linear.y>.8f||input.motion.angular.dotproduct(raw.i)<-3.0f))input.closing=true;
    }
    if(kind==2&&input.closing) {
        gun->VRInternalBreak(_max(0.f,state.angle-Device.fTimeDelta*3.f));
        if(state.angle<=.01f){input.closing=false;pulse(primaryHand(),.35f,25);}
    }
    if(!held&&((kind==1&&state.pump<=.08f)||(kind==2&&state.angle<=.03f)))articulatedSupport().aimLocked=false;
}
}
