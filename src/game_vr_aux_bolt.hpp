#pragma once
#include "game_vr_controls.hpp"
#include "game_vr_motion.hpp"
#include "xrGame/Actor.h"
#include "xrGame/Inventory.h"
#include "xrGame/Bolt.h"
#include "xrGame/Level.h"
namespace copvr_game {
struct AuxBolt {u16 item=u16(-1);int side=-1;bool pending=false;u32 until=0;Fmatrix pose;Fvector linear{},angular{};MotionHistory motion;};
inline AuxBolt& auxBolt(){static AuxBolt value;return value;}
inline void resetAuxBolt(){auto& state=auxBolt();if(state.side>=0){occupiedHands()[state.side]=false;heldObjectRadius()[state.side]=0;}state=AuxBolt{};}
inline bool beginAuxBolt(CActor& actor,int side,const Fmatrix& pose){
    auto bolt=smart_cast<CBolt*>(actor.inventory().ItemFromSlot(BOLT_SLOT));
    if(!bolt||side==primaryHand()||occupiedHands()[side]||auxBolt().side>=0)return false;
    auto& state=auxBolt();state.item=bolt->ID();state.side=side;state.pose=pose;
    occupiedHands()[side]=claimedHands()[side]=true;heldObjectRadius()[side]=.025f;pulse(side,.3f,30);return true;
}
inline void updateAuxBolt(CActor& actor,const CopVrControls& controls,const Fmatrix& camera,bool allowed){
    auto& state=auxBolt();if(state.side<0)return;
    auto bolt=smart_cast<CBolt*>(Level().Objects.net_Find(state.item));Fmatrix pose;Fvector grip;
    if(!allowed||!bolt||bolt->H_Parent()!=&actor||state.side==primaryHand()||!hand(controls,state.side,camera,pose,grip)){resetAuxBolt();return;}
    const int side=state.side;occupiedHands()[side]=claimedHands()[side]=true;heldObjectRadius()[side]=.025f;
    pose.c=trackedPalm(pose,grip,side);
    if(!state.pending){
        state.pose=pose;state.motion.update(unsmoothedHandPose(pose,camera,side),Device.fTimeDelta);
        if(controls.hands[side].squeeze<.25f){state.pending=true;state.until=Device.dwTimeGlobal+2000;state.linear=state.motion.throwLinear();state.angular=state.motion.angular;}
    }
    if(state.pending&&(bolt->ThrowVR(state.pose,state.linear,state.angular)||Device.dwTimeGlobal>state.until)){pulse(side,.25f,25);resetAuxBolt();}
}
}
