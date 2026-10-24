#pragma once
#include "game_vr_controls.hpp"
#include "xrGame/ActorEffector.h"
#include "xrGame/Hit.h"
#include "xrGame/Inventory.h"
#include "xrGame/MainMenu.h"
#include "xrGame/UIGameCustom.h"
#include "xrNetServer/NET_Messages.h"
// Included by WeaponKnife.cpp: member implementation can use native knife damage and sound.
void CWeaponKnife::UpdateVRMelee(const Fmatrix& itemPose,const Fmatrix& trackingBase,IKinematics* model,bool poseReady) {
    auto reset=[&]() { m_vrSampleTime=0; m_vrSwing.reset(); m_vrHand=-1; };
    auto actor=smart_cast<CActor*>(H_Parent()); CopVrControls controls{};
    const int side=copvr_game::primaryHand();
    if(!strstr(Core.Params,"-copvr")||!IsGameTypeSingle()||!poseReady||!model||!actor||!actor->g_Alive()||
        !OnServer()||actor!=Level().CurrentViewEntity()||actor!=Level().CurrentControlEntity()||
        actor->inventory().ActiveItem()!=this||GetState()!=eIdle||actor->inventory().GetNextActiveSlot()!=BaseSlot()||Device.Paused()||actor->IsTalking()||
        MainMenu()->IsActive()||!CurrentGameUI()||CurrentGameUI()->TopInputReceiver()||
        !copvr_game::sample(controls)||!controls.active||!controls.hands[side].valid) { reset(); return; }
    if(controls.timestampMs==m_vrSampleTime) return;
    Fmatrix rawItem=copvr_game::unsmoothedHandPose(itemPose,trackingBase,side);
    Fmatrix pose=rawItem; const u16 blade=model->LL_BoneID("knife");
    if(blade!=BI_NONE) {
        xr_vector<Fmatrix> bind; model->LL_GetBindTransform(bind);
        if(blade>=bind.size()) { reset(); return; }
        Fmatrix inverseBind,delta; inverseBind.invert(bind[blade]);
        delta.mul_43(model->LL_GetTransform(blade),inverseBind); pose.mul_43(rawItem,delta);
    }
    Fmatrix inverseBase,tracked; inverseBase.invert(trackingBase); tracked.mul_43(inverseBase,pose);
    const float dt=m_vrSampleTime&&controls.timestampMs>m_vrSampleTime?float(controls.timestampMs-m_vrSampleTime)*.001f:0;
    const Fvector heel=Fvector().set(0,.004f,.030f),tip=Fvector().set(.000083f,.004791f,.170585f);
    Fvector oldTip,newTip,oldHeel,newHeel;
    m_vrPreviousTracking.transform_tiny(oldTip,tip); tracked.transform_tiny(newTip,tip);
    m_vrPreviousTracking.transform_tiny(oldHeel,heel); tracked.transform_tiny(newHeel,heel);
    const bool continuous=m_vrSampleTime&&m_vrHand==side&&dt>.001f&&dt<=.1f;
    Fvector velocity; velocity.set(0,0,0); float speed=0;
    if(continuous) {
        Fvector heelVelocity; velocity.sub(newTip,oldTip); velocity.div(dt);
        heelVelocity.sub(newHeel,oldHeel); heelVelocity.div(dt);
        if(heelVelocity.square_magnitude()>velocity.square_magnitude()) velocity=heelVelocity;
        speed=velocity.magnitude();
    }
    Fvector worldOld,worldNew; m_vrPreviousWorld.transform_tiny(worldOld,tip); pose.transform_tiny(worldNew,tip);
    const bool safe=continuous&&speed<=12.f&&worldOld.distance_to(worldNew)<.45f;
    const bool strike=safe&&m_vrSwing.update({velocity.x,velocity.y,velocity.z},dt);
    if(!safe) m_vrSwing.reset();
    if(strike) {
        collide::rq_result best{}; Fvector contact{},direction{}; bool found=false; float fraction=2.f;
        auto trace=[&](const Fvector& from,const Fvector& to) {
            Fvector ray; ray.sub(to,from); const float distance=ray.magnitude(); if(distance<.0001f) return;
            ray.div(distance); collide::rq_result hit;
            if(!Level().ObjectSpace.RayPick(from,ray,distance,collide::rqtBoth,hit,actor)) return;
            const float t=hit.range/distance;
            if(t<fraction) { fraction=t; best=hit; contact=from; contact.mad(ray,hit.range); direction=ray; found=true; }
        };
        // Sweep the blade length and a narrow edge band, rather than an actor-facing attack cone.
        for(int edge=-1;edge<=1;++edge) for(int sample=0;sample<=6;++sample) {
            Fvector local; local.lerp(heel,tip,float(sample)/6); local.y+=edge*.006f;
            Fvector from,to; m_vrPreviousWorld.transform_tiny(from,local); pose.transform_tiny(to,local); trace(from,to);
        }
        Fvector a,b; pose.transform_tiny(a,heel); pose.transform_tiny(b,tip); trace(a,b);
        if(found) {
            // A target reached through a wall/another object cannot receive the hit.
            Fvector handle=pose.c,ray; ray.sub(contact,handle); float distance=ray.magnitude();
            bool visible=true;
            if(distance>.001f) {
                ray.div(distance); collide::rq_result blocker;
                if(Level().ObjectSpace.RayPick(handle,ray,distance,collide::rqtBoth,blocker,actor)&&blocker.O!=best.O&&blocker.range<distance-.005f) visible=false;
            }
            m_vrSwing.contact({velocity.x,velocity.y,velocity.z});
            copvr_game::pulse(side,.4f,40); PlaySound("sndShot",contact);
            if(visible&&best.O&&best.O!=actor) {
                auto target=smart_cast<CGameObject*>(best.O);
                if(target&&!target->getDestroy()) {
                    Fmatrix inverse; inverse.invert(target->XFORM()); Fvector local; inverse.transform_tiny(local,contact);
                    u16 bone=BI_NONE; auto visual=target->Visual(); auto skeleton=visual?visual->dcast_PKinematics():nullptr;
                    if(skeleton&&best.element<skeleton->LL_BoneCount()) {
                        bone=u16(best.element); inverse.invert(skeleton->LL_GetTransform(bone)); Fvector objectPoint=local; inverse.transform_tiny(local,objectPoint);
                    }
                    trackingBase.transform_dir(direction,velocity); direction.normalize_safe();
                    const float scale=copvr::knifeDamageScale(speed);
                    const float power=fvHitPower_1[g_SingleGameDifficulty]*scale;
                    SHit hit(power,direction,actor,bone,local,fHitImpulse_1*scale,m_eHitType_1,0.f,false);
                    hit.GenHeader(GE_HIT,target->ID()); hit.whoID=actor->ID(); hit.weaponID=ID();
                    NET_Packet packet; hit.Write_Packet(packet); u_EventSend(packet);
                    Msg("* CoPVR knife: contact target=%u hand=%d speed=%.2f scale=%.2f",target->ID(),side,speed,scale);
                }
            }
        }
    }
    m_vrPreviousWorld=pose; m_vrPreviousTracking=tracked; m_vrSampleTime=controls.timestampMs; m_vrHand=side;
}
