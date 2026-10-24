#pragma once
#include "vr_bridge_api.h"
#include "two_hand.hpp"
#include "weapon_contact.hpp"
#include "game_vr_equipment_config.hpp"
#include "game_vr_extra_state.hpp"
#include "game_vr_hand_filter.hpp"
namespace copvr_game {
static_assert(sizeof(CopVrHand)==60&&sizeof(CopVrControls)==136,"Unexpected controls ABI packing");
inline void pulse(uint32_t hand=1,float amplitude=.4f,uint32_t duration=25) {
#if defined(XR_PLATFORM_WINDOWS)
    if(auto library=GetModuleHandleA("copvr_bridge.dll"))
        if(auto feedback=reinterpret_cast<CopVrPulse>(GetProcAddress(library,"copvr_pulse"))) feedback(hand,_max(.35f,_min(1.f,amplitude*2.5f)),_max(55u,duration*2));
#endif
}
inline bool sample(CopVrControls& value) {
    value={}; value.abi=COPVR_CONTROLS_ABI;
#if defined(XR_PLATFORM_WINDOWS)
    if(!strstr(Core.Params,"-copvr")) return false;
    if(auto library=GetModuleHandleA("copvr_bridge.dll")) {
        auto get=reinterpret_cast<CopVrGetControls>(GetProcAddress(library,"copvr_get_controls"));
        const bool available=get&&get(&value)>0&&value.abi==COPVR_CONTROLS_ABI;
        if(!available||!value.active) for(int side=0;side<2;++side) {
            handFilters()[side].filter.reset(); handFilters()[side].ready=false;
        }
        if(available) {
            static u32 calibrationSerial=0;CopVrHeight height{};height.abi=COPVR_HEIGHT_ABI;
            auto getHeight=reinterpret_cast<CopVrGetHeight>(GetProcAddress(library,"copvr_get_height"));
            if(getHeight&&getHeight(&height)>0&&height.serial!=calibrationSerial) {
                calibrationSerial=height.serial;
                for(int side=0;side<2;++side){handFilters()[side].filter.reset();handFilters()[side].ready=false;}
            }
        }
        return available;
    }
#endif
    return false;
}
struct TrackedGrip { Fmatrix pose; bool valid=false; };
inline TrackedGrip* trackedGrips() { static TrackedGrip value[2]; return value; }
inline bool hand(const CopVrControls& controls,int index,const Fmatrix& camera,Fmatrix& aim,Fvector& grip,bool useGrip=false) {
    if(index<0||index>1) return false;
    auto invalid=[&]() { handFilters()[index].filter.reset(); handFilters()[index].ready=false; trackedGrips()[index].valid=false; return false; };
    const auto& hand=controls.hands[index];
    if(!controls.active||!hand.valid) return invalid();
    Fvector direction,up,right,position,localGrip;
    direction.set(hand.aimDirection[0],hand.aimDirection[1],hand.aimDirection[2]);
    up.set(hand.aimUp[0],hand.aimUp[1],hand.aimUp[2]);
    position.set(hand.aimPosition[0],hand.aimPosition[1],hand.aimPosition[2]);
    localGrip.set(hand.gripPosition[0],hand.gripPosition[1],hand.gripPosition[2]);
    if(!_valid(direction)||!_valid(up)||!_valid(position)||!_valid(localGrip)||
        direction.square_magnitude()<.01f||up.square_magnitude()<.01f) return invalid();
    right.crossproduct(up,direction); right.normalize_safe();
    if(right.square_magnitude()<.5f) return invalid();
    up.crossproduct(direction,right); up.normalize_safe(); direction.normalize_safe();
    Fmatrix local; local.set(right,up,direction,position); aim.mul_43(camera,local);
    camera.transform_tiny(grip,localGrip);
    auto& tracked=trackedGrips()[index]; tracked.valid=false;
#if defined(XR_PLATFORM_WINDOWS)
    CopVrGrips poses{}; poses.abi=COPVR_GRIP_ABI;
    auto library=GetModuleHandleA("copvr_bridge.dll");
    auto get=library?reinterpret_cast<CopVrGetGrips>(GetProcAddress(library,"copvr_get_grips")):nullptr;
    if(get&&get(&poses)>0&&poses.timestampMs==controls.timestampMs&&poses.hands[index].valid) {
        const auto& pose=poses.hands[index];
        direction.set(pose.direction[0],pose.direction[1],pose.direction[2]);
        up.set(pose.up[0],pose.up[1],pose.up[2]);
        if(_valid(direction)&&_valid(up)&&direction.square_magnitude()>.5f&&up.square_magnitude()>.5f) {
            direction.normalize_safe(); right.crossproduct(up,direction); right.normalize_safe();
            if(right.square_magnitude()>.5f) {
                up.crossproduct(direction,right); up.normalize_safe();
                local.set(right,up,direction,localGrip); tracked.pose.mul_43(camera,local); tracked.valid=true;
                if(useGrip) aim=tracked.pose;
            }
        }
    }
#endif
    Fmatrix rawGrip;
    if(tracked.valid) { Fmatrix inverse; inverse.invert(camera); rawGrip.mul_43(inverse,tracked.pose); }
    else { rawGrip=local; rawGrip.c=localGrip; }
    const Fmatrix filtered=smoothHandGrip(index,rawGrip,controls.timestampMs);
    Fmatrix inverseRaw,localDelta,worldDelta,inverseCamera,remap;
    inverseRaw.invert(rawGrip); localDelta.mul_43(filtered,inverseRaw);
    inverseCamera.invert(camera); remap.mul_43(camera,localDelta); worldDelta.mul_43(remap,inverseCamera);
    Fmatrix unfiltered=aim; aim.mul_43(worldDelta,unfiltered);
    Fvector oldGrip=grip; worldDelta.transform_tiny(grip,oldGrip);
    if(tracked.valid) { unfiltered=tracked.pose; tracked.pose.mul_43(worldDelta,unfiltered); }
    return true;
}
struct ArticulatedSupport {
    const void* weapon=nullptr;bool pointValid=false,aimLocked=false;
    Fvector point{};Fmatrix aimOffset;
};
inline ArticulatedSupport& articulatedSupport() { static ArticulatedSupport value;return value; }
inline copvr::SupportGrip& supportGrip() { static copvr::SupportGrip state; return state; }
inline int& primaryHand() { static int value=1; return value; }
inline int secondaryHand() { return 1-primaryHand(); }
inline bool* occupiedHands() { static bool value[2]{}; return value; }
inline bool* nativeHeldHands() { static bool value[2]{}; return value; }
struct HudPoseReady { const void* owner{}; bool ready=false; };
inline HudPoseReady& hudPoseReady() { static HudPoseReady state; return state; }
struct NativeWrist { Fmatrix pose; bool valid=false; int source=1; };
inline NativeWrist* nativeWrists() { static NativeWrist value[2]; return value; }
inline bool* claimedHands() { static bool value[2]{}; return value; }
struct PartGripAnchor { bool active=false;int side=0;Fvector palm{}; };
inline PartGripAnchor& partGripAnchor() { static PartGripAnchor value;return value; }
inline bool& manipulatingWeapon() { static bool value=false; return value; }
inline float* heldObjectRadius() { static float value[2]{}; return value; }
struct InteractionZones { bool use=false,stow=false,reload=false,holster=false; Fvector port{},holsterPoint{}; };
inline InteractionZones* interactionZones() { static InteractionZones value[2]; return value; }
struct PalmSocket { Fvector aimOffset{},normal{},boneLocal{}; Fmatrix wristFromGrip; bool valid=false; };
inline PalmSocket* palmSockets() { static PalmSocket value[2]; return value; }
inline Fvector trackedPalm(const Fmatrix& aim,const Fvector& wrist,int side) {
    Fvector value=wrist,offset;
    const Fmatrix& orientation=trackedGrips()[side].valid?trackedGrips()[side].pose:aim;
    if(palmSockets()[side].valid) { orientation.transform_dir(offset,palmSockets()[side].aimOffset); value.add(offset); }
    return value;
}
inline bool& weaponTransferLatched() { static bool value=false; return value; }
inline void updateWeaponHand(const CopVrControls& controls,const Fmatrix& camera,bool enabled) {
    auto& latched=weaponTransferLatched();
    const int owner=primaryHand(),other=1-owner;
    if(!enabled||!controls.active) { latched=false; return; }
    Fmatrix a,b; Fvector p,q;
    if(!hand(controls,owner,camera,a,p)||!hand(controls,other,camera,b,q)||occupiedHands()[other]) { latched=false; return; }
    if(controls.hands[owner].squeeze>.7f&&controls.hands[other].squeeze>.7f&&p.distance_to(q)<.09f) latched=true;
    if(latched&&p.distance_to(q)>.16f) latched=false;
    if(latched&&controls.hands[other].squeeze<.25f) latched=false;
    if(latched&&controls.hands[owner].squeeze<.25f) {
        primaryHand()=other; supportGrip().reset(); latched=false;
        pulse(other,.3f,30); Msg("* CoPVR: weapon transferred to hand %d",other);
    }
}
struct GripProfile { const void* weapon{}; bool valid{},locked{},mirrored{},primaryMirrored{},supportEnabled{}; Fvector right,foregrip; Fmatrix left,rightSupport,worldLeft,leftPrimary,rightPrimary,worldPrimary; };
inline GripProfile& gripProfile() { static GripProfile value; return value; }
inline void resetSupportGrip() { supportGrip().reset(); gripProfile().locked=false; }
inline void configureGrip(const void* weapon,const Fvector& right,const Fmatrix& left,const Fmatrix& rightSupport,bool mirrored,bool enabled) {
    auto& p=gripProfile();
    if(p.weapon!=weapon||!supportGrip().active()) {
        p.weapon=weapon; p.right=right; p.left=left; p.rightSupport=rightSupport; p.mirrored=mirrored; p.valid=true;
        if(palmSockets()[secondaryHand()].valid) (secondaryHand()==0?left:rightSupport).transform_tiny(p.foregrip,palmSockets()[secondaryHand()].boneLocal);
        else p.foregrip=left.c;
    }
    p.supportEnabled=enabled;
    if(!enabled) resetSupportGrip();
}
inline bool lockedSupport(Fmatrix& wrist) {
    auto& p=gripProfile(); if(!p.locked||!supportGrip().active()) return false;
    wrist=p.worldLeft; return true;
}
inline bool rightHand(const CopVrControls& controls,const Fmatrix& camera,Fmatrix& aim,Fvector& grip,const void* weapon) {
    static const void* previousWeapon{};
    auto& state=supportGrip();
    if(previousWeapon!=weapon) { resetSupportGrip(); previousWeapon=weapon; }
    // Runtime aim is the validated barrel frame. Grip supplies the physical
    // contact point, not a generic open-palm orientation for every gun.
    if(!hand(controls,primaryHand(),camera,aim,grip)) { resetSupportGrip(); return false; }
    auto& profile=gripProfile();auto& articulation=articulatedSupport();
    const bool fixedAim=articulation.weapon==weapon&&articulation.aimLocked;
    if(fixedAim) { Fmatrix pointing=aim;aim.mul_43(pointing,articulation.aimOffset);aim.c=pointing.c; }
    const int owner=primaryHand();
    const Fvector palmPosition=trackedPalm(aim,grip,owner);
    const bool matching=profile.valid&&profile.weapon==weapon;
    // Keep the authored fist on the handle by moving the wrist with the item
    // attachment below. Inverting its rotation against an open bind-pose hand
    // introduced a large pitch error (forward Touch produced an upright gun).
    grip=palmPosition;
    const int offhand=secondaryHand();
    Fmatrix left; Fvector leftGrip; leftGrip.set(0,0,0);
    const bool tracked=hand(controls,offhand,camera,left,leftGrip);
    if(tracked) leftGrip=trackedPalm(left,leftGrip,offhand);
    const bool wasActive=state.active();
    profile.locked=false;
    Fvector wristOffset,foregrip; foregrip=grip;
    if(matching) {
        aim.transform_dir(wristOffset,profile.right);
        Fmatrix oneHand=aim; oneHand.c.sub(grip,wristOffset);
        oneHand.transform_tiny(foregrip,articulation.weapon==weapon&&articulation.pointValid?articulation.point:profile.foregrip);
    }
    const bool active=state.update(tracked&&!occupiedHands()[offhand]&&!claimedHands()[offhand]&&profile.valid&&profile.supportEnabled&&profile.weapon==weapon&&
        (offhand==0||profile.mirrored),{grip.x,grip.y,grip.z},{leftGrip.x,leftGrip.y,leftGrip.z},
        {aim.k.x,aim.k.y,aim.k.z},controls.hands[offhand].squeeze,leftGrip.distance_to(foregrip)<.11f);
    if(active) {
        // Restore the previously validated controller-to-controller aim.
        // Locking the offhand pose must not reinterpret authored wrist axes
        // as a correction to the weapon's forward direction.
        Fvector direction,right,up; direction.sub(leftGrip,grip); direction.normalize_safe();
        if(fixedAim)direction=aim.k;
        right.crossproduct(aim.j,direction); right.normalize_safe();
        if(right.square_magnitude()<.5f) right=aim.i;
        up.crossproduct(direction,right); up.normalize_safe();
        aim.i=right; aim.j=up; aim.k=direction;
        Fvector offset; aim.transform_dir(offset,profile.right);
        Fmatrix weaponPose=aim; weaponPose.c.sub(grip,offset);
        profile.worldLeft.mul_43(weaponPose,offhand==0?profile.left:profile.rightSupport); profile.locked=true;
    }
    if(active!=wasActive) { Msg("* CoPVR: support grip=%d hand=%d",active,offhand); if(active) pulse(offhand,.2f,20); }
    if(matching&&(primaryHand()==1||profile.primaryMirrored)) {
        const auto orientation=copvr::basisQuaternion({aim.i.x,aim.i.y,aim.i.z},{aim.j.x,aim.j.y,aim.j.z},{aim.k.x,aim.k.y,aim.k.z});
        const copvr::Pose pointing{{aim.c.x,aim.c.y,aim.c.z},orientation,true};
        const copvr::Pose contact{{grip.x,grip.y,grip.z},{},true};
        const auto object=copvr::weaponAtPalm(pointing,contact,{profile.right.x,profile.right.y,profile.right.z});
        Fmatrix weaponPose=aim; weaponPose.c.set(object.position.x,object.position.y,object.position.z);
        profile.worldPrimary.mul_43(weaponPose,primaryHand()==0?profile.leftPrimary:profile.rightPrimary);
    }
    return true;
}
}
