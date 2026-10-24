#pragma once
#include "game_vr_controls.hpp"
#include "game_vr_motion.hpp"
#include "grenade_primer.hpp"
#include "xrGame/Bolt.h"
#include "xrGame/Grenade.h"
#include "xrGame/Inventory.h"

namespace copvr_game {
struct TrackedToolState {
    u16 id=u16(-1);
    bool held=false;
    bool pending=false;
    bool primed=false;
    copvr::GrenadePrimer primer;
    u32 until=0;
    Fmatrix releasePose;
    Fvector releaseLinear,releaseAngular;
    MotionHistory motion;
};
inline TrackedToolState& trackedToolState() { static TrackedToolState value; return value; }
inline void resetTrackedTools() { trackedToolState()=TrackedToolState{}; }
inline bool updateTrackedTools(CActor& actor,const CopVrControls& controls,const Fmatrix& camera,bool allowed) {
    auto& state=trackedToolState(); auto active=actor.inventory().ActiveItem();
    auto bolt=smart_cast<CBolt*>(active); auto grenade=smart_cast<CGrenade*>(active);
    const int side=primaryHand(); Fmatrix pose; Fvector grip;
    if(!allowed||(!bolt&&!grenade)||actor.inventory().GetNextActiveSlot()!=(grenade?GRENADE_SLOT:BOLT_SLOT)||
        !hand(controls,side,camera,pose,grip)) {
        if(grenade&&!allowed) grenade->CancelPrimeVR();
        resetTrackedTools(); return false;
    }
    pose.c=grip;
    if(state.id!=active->object().ID()) {
        resetTrackedTools(); state.id=active->object().ID();
        state.held=grenade&&bodyEquipmentState().primeGrenadeHand==side;
    }
    state.motion.update(unsmoothedHandPose(pose,camera,side),Device.fTimeDelta);
    const float squeeze=controls.hands[side].squeeze;
    if(!state.pending&&squeeze>.7f) state.held=true;
    if(grenade&&!state.pending&&state.held&&squeeze>.7f&&!state.primed&&bodyEquipmentState().primeGrenadeHand==side) {
        if(grenade->PrimeVR()) {
            state.primed=true; bodyEquipmentState().primeGrenadeHand=-1; pulse(side,.4f,45);
        }
    }
    if(grenade&&!state.pending&&state.held&&!state.primed) {
        const int other=1-side; Fmatrix otherPose; Fvector otherGrip,relative; relative.set(0,0,0);
        const bool free=!occupiedHands()[other]&&!nativeHeldHands()[other]&&!claimedHands()[other]&&
            hand(controls,other,camera,otherPose,otherGrip);
        if(free) relative.sub(otherGrip,grip);
        if(state.primer.update(squeeze,controls.hands[side].trigger,free,controls.hands[other].squeeze,{relative.x,relative.y,relative.z})) {
            if(grenade->PrimeVR()) { state.primed=true; pulse(side,.35f,45); if(free) pulse(other,.25f,30); }
            else state.primer=copvr::GrenadePrimer{};
        }
    }
    if(!state.pending&&state.held&&squeeze<.25f) {
        if(grenade&&inventoryStowZone(camera,trackedPalm(pose,grip,side))) {
            grenade->CancelPrimeVR(); bodyEquipmentState().primeGrenadeHand=-1;
            actor.inventory().Activate(NO_ACTIVE_SLOT); bodyEquipmentState().hands[side].index=-1;
            pulse(side,.25f,35); resetTrackedTools(); return true;
        }

        if(!state.primed&&inventoryStowZone(camera,trackedPalm(pose,grip,side))) {
            actor.inventory().Activate(NO_ACTIVE_SLOT); bodyEquipmentState().hands[side].index=-1;
            pulse(side,.2f,25); resetTrackedTools(); return true;
        }
        if(grenade&&!state.primed) {
            grenade->CancelPrimeVR(); bodyEquipmentState().primeGrenadeHand=-1;
            actor.inventory().Activate(NO_ACTIVE_SLOT); bodyEquipmentState().hands[side].index=-1;
            pulse(side,.2f,25); resetTrackedTools(); return true;
        }
        state.held=false; state.pending=true; state.until=Device.dwTimeGlobal+2000;
        state.releasePose=pose; state.releaseLinear=state.motion.throwLinear(); state.releaseAngular=state.motion.angular;
        Msg("* CoPVR: %s release queued hand=%d speed=%f",grenade?"grenade":"bolt",side,state.releaseLinear.magnitude());
    }
    if(!state.pending) return false;
    if(Device.dwTimeGlobal>state.until) {
        Msg("! CoPVR: tracked throw timed out id=%u",state.id);
        if(grenade) { actor.inventory().Activate(NO_ACTIVE_SLOT); bodyEquipmentState().hands[side].index=-1; }
        resetTrackedTools(); return false;
    }
    if(!(grenade?grenade->ThrowVR(state.releasePose,state.releaseLinear,state.releaseAngular):bolt->ThrowVR(state.releasePose,state.releaseLinear,state.releaseAngular))) return false;
    actor.inventory().Activate(NO_ACTIVE_SLOT); pulse(side,.25f,25);
    bodyEquipmentState().hands[side].index=-1;
    Msg("* CoPVR: tracked %s release hand=%d speed=%f",grenade?"grenade":"bolt",side,state.releaseLinear.magnitude());
    resetTrackedTools(); return true;
}
}
