#pragma once
#include "game_vr_controls.hpp"
#include "game_vr_aux_bolt.hpp"
#include "game_vr_magazine.hpp"
#include "game_vr_internal_reload.hpp"
#include "game_vr_reload_grip.hpp"
#include "game_vr_posture.hpp"
#include "body_pose_estimator.hpp"
#include "xrGame/Actor.h"
#include "xrGame/Inventory.h"
#include "xrGame/Weapon.h"
#include "xrGame/CustomDetector.h"
#include "xrGame/WeaponPistol.h"
#include "xrGame/WeaponKnife.h"
#include "xrGame/WeaponAmmo.h"
#include "xrGame/Missile.h"
#include "xrGame/Grenade.h"
#include "xrGame/player_hud.h"

namespace copvr_game {
inline bool equipExtraWeapon(CActor&,CWeapon&,int);
inline bool equipExtraTool(CActor&,CMissile&,int);
struct BodyHeading { copvr::TorsoHeading torso; u32 frame=0; float yaw=0,movementYaw=0; bool moving=false; };
inline BodyHeading& bodyHeading() { static BodyHeading value; return value; }
inline void alignBodyMovement(const Fvector& velocity,const Fmatrix& camera,bool allowed) {
    auto& heading=bodyHeading(); Fvector forward=camera.k,up,right; up.set(0,1,0);
    forward.y=0; forward.normalize_safe(); right.crossproduct(up,forward);
    const float x=velocity.dotproduct(right),z=velocity.dotproduct(forward);
    heading.moving=allowed&&x*x+z*z>.09f;
    if(heading.moving) heading.movementYaw=atan2f(x,z);
}
struct BodyPoseState {
    copvr::BodyPoseEstimator estimator; copvr::EstimatedBodyPose pose;
    u32 frame=u32(-1),serial=0; const void* owner{};
};
inline BodyPoseState& bodyPoseState() { static BodyPoseState value; return value; }
inline bool estimatedBodyFrame(const Fmatrix& camera,Fmatrix& frame,bool chest) {
#if defined(XR_PLATFORM_WINDOWS)
    CopVrHead head{}; if(!postureHead(head)) return false;
    auto library=GetModuleHandleA("copvr_bridge.dll");
    CopVrHeight height{}; height.abi=COPVR_HEIGHT_ABI;
    auto getHeight=library?reinterpret_cast<CopVrGetHeight>(GetProcAddress(library,"copvr_get_height")):nullptr;
    const bool heightValid=getHeight&&getHeight(&height)>0;
    auto& heading=bodyHeading(); auto& state=bodyPoseState();
    if(heightValid&&state.serial!=height.serial)heading=BodyHeading{};
    if(heading.frame!=Device.dwFrame) {
        const float horizontal=head.direction[0]*head.direction[0]+head.direction[2]*head.direction[2];
        if(heading.moving&&horizontal>.04f) heading.yaw=heading.torso.alignWithinView(heading.movementYaw,atan2f(head.direction[0],head.direction[2]),Device.fTimeDelta);
        else if(horizontal>.04f) heading.yaw=heading.torso.update(atan2f(head.direction[0],head.direction[2]),Device.fTimeDelta);
        heading.frame=Device.dwFrame;
    }
    const void* owner=g_pGameLevel?Level().CurrentViewEntity():nullptr;
    const bool reset=state.owner!=owner||(heightValid&&state.serial!=height.serial);
    if(state.frame!=Device.dwFrame||reset) {
        state.pose=state.estimator.update({head.position[0],head.position[1],head.position[2]},
            {head.direction[0],head.direction[1],head.direction[2]},{head.up[0],head.up[1],head.up[2]},
            heading.yaw,heightValid?height.standingEyeHeight:1.65f,Device.fTimeDelta,reset);
        state.owner=owner;state.frame=Device.dwFrame;if(heightValid)state.serial=height.serial;
    }
    Fvector up,baseForward=camera.k,baseRight;up.set(0,1,0);
    baseForward.y=0;baseForward.normalize_safe();baseRight.crossproduct(up,baseForward);baseRight.normalize_safe();
    auto direction=[&](copvr::Vec3 p) { Fvector v;v.mul(baseRight,p.x);v.mad(up,p.y);v.mad(baseForward,p.z);return v; };
    Fvector forward;forward.mul(baseForward,cosf(heading.yaw));forward.mad(baseRight,sinf(heading.yaw));
    Fvector position;
    if(chest) {
        const auto centre=direction(state.pose.chestCenter);up=direction(state.pose.chestUp);up.normalize_safe();
        position.add(camera.c,centre);position.mad(up,-copvr::chestHeight);
        const float along=forward.dotproduct(up);forward.mad(up,-along);forward.normalize_safe();
    } else position.add(camera.c,direction(state.pose.waistOrigin));
    Fvector right;right.crossproduct(up,forward);right.normalize_safe();
    if(right.square_magnitude()<.5f||!_valid(position))return false;
    frame.set(right,up,forward,position);frame=equipmentPose("body","frame",frame);return true;
#else
    return false;
#endif
}
inline bool bodyFrame(const Fmatrix& camera,Fmatrix& frame) { return estimatedBodyFrame(camera,frame,false); }
inline bool chestFrame(const Fmatrix& camera,Fmatrix& frame) { return estimatedBodyFrame(camera,frame,true); }

// Stable visual anchors. Slot 2 is left, slot 3 right; type selects hip/shoulder.
inline Fvector weaponAnchor(CInventoryItem* item,u16 slot) {
    const float side=slot==INV_SLOT_2?-1.f:1.f;
    const bool pistol=smart_cast<CWeaponPistol*>(item)!=nullptr;
    const char* key=pistol?(side<0?"left_hip":"right_hip"):(side<0?"left_shoulder":"right_shoulder");
    return equipmentVector("body",key,pistol?Fvector().set(side*.39f,-.88f,-.12f):Fvector().set(side*.27f,-.16f,-.25f));
}
inline bool headGestureFrame(const Fmatrix& camera,Fmatrix& frame) {
    CopVrHead head{}; if(!postureHead(head)) return false;
    Fvector position,forward,up,right;
    position.set(head.position[0],head.position[1],head.position[2]); camera.transform_tiny(position);
    forward.set(head.direction[0],head.direction[1],head.direction[2]); camera.transform_dir(forward);
    forward.y=0; forward.normalize_safe(); if(forward.square_magnitude()<.5f) return false;
    up.set(0,1,0); right.crossproduct(up,forward); frame.set(right,up,forward,position); return true;
}
inline bool headWeaponZone(const Fmatrix& camera,const Fvector& palm,bool returning) {
    Fmatrix frame,inverse; if(!headGestureFrame(camera,frame)) return false;
    inverse.invert(frame); Fvector local; inverse.transform_tiny(local,palm);
    return copvr::headWeaponVolume(local.x,local.y,local.z,returning);
}
inline u16 headWeaponSlot(CActor& actor,const Fmatrix& camera,const Fvector& palm,int side) {
    const u16 slot=side==0?INV_SLOT_2:INV_SLOT_3;
    auto item=actor.inventory().ItemFromSlot(slot);
    if(!item||item==actor.inventory().ActiveItem()||isExtraWeapon(smart_cast<CWeapon*>(item))) return NO_ACTIVE_SLOT;
    return headWeaponZone(camera,palm,false)?slot:NO_ACTIVE_SLOT;
}
inline Fvector chestZoneCenter() { return Fvector().set(0,copvr::chestHeight,copvr::chestForward); }
inline Fvector chestZoneHalf() { return Fvector().set(copvr::chestHalfX,copvr::chestHalfY,copvr::chestHalfZ); }
inline Fvector backToolsCenter() { return Fvector().set(0,copvr::backToolsHeight,copvr::backToolsDepth); }
inline Fvector backToolsHalf() { return Fvector().set(copvr::backToolsHalfX,copvr::backToolsHalfY,copvr::backToolsHalfZ); }
inline bool chestZoneLocal(const Fvector& local) { return copvr::chestVolume(local.x,local.y,local.z); }
inline bool backToolsZoneLocal(const Fvector& local) {
    return copvr::backToolsVolume(local.x,local.y,local.z);
}
inline bool inventoryStowZone(const Fmatrix& camera,const Fvector& palm) {
    Fmatrix body,inverse; if(!chestFrame(camera,body)) return false;
    inverse.invert(body); Fvector local; inverse.transform_tiny(local,palm);
    return chestZoneLocal(local);
}
inline bool torchTempleZone(const Fmatrix& camera,const Fvector& palm) {
    Fmatrix frame,inverse; if(!headGestureFrame(camera,frame)) return false;
    inverse.invert(frame); Fvector local; inverse.transform_tiny(local,palm);
    return copvr::templeVolume(local.x,local.y,local.z);
}

enum BeltIndex { BeltKnife,BeltBinocular,BeltDetector,BeltBolt,BeltAmmo,BeltQuick0,BeltQuick1,BeltQuick2,BeltQuick3,BeltGrenade,BeltCount };
inline Fvector beltAnchor(int index) {
    if(index==BeltGrenade) return equipmentVector("body","grenade",Fvector().set(-.34f,-.63f,0.f));
    if(index==BeltAmmo) return backToolsCenter();
    // Waist tools and lower quick pockets, close to the torso and below view.
    const Fvector anchors[]={Fvector().set(-.27f,-.61f,.13f),Fvector().set(-.09f,-.61f,.20f),
        Fvector().set(.09f,-.61f,.20f),Fvector().set(.27f,-.61f,.13f),Fvector().set(0,-.63f,.06f),
        Fvector().set(-.30f,-.83f,.05f),Fvector().set(-.10f,-.83f,.14f),
        Fvector().set(.10f,-.83f,.14f),Fvector().set(.30f,-.83f,.05f)};
    string32 key; xr_sprintf(key,"belt_%d",index);
    return equipmentVector("body",key,anchors[index]);
}
inline float beltReach(const Fvector& palm,int index) {
    Fvector delta; delta.sub(palm,beltAnchor(index));
    // A hand can approach from the front without finding a tiny center point.
    const Fvector half=index==BeltAmmo?Fvector().set(.11f,.12f,.12f):Fvector().set(.10f,.095f,.13f);
    return _max(_abs(delta.x)/half.x,_max(_abs(delta.y)/half.y,_abs(delta.z)/half.z));
}
inline u16 beltNativeSlot(int index) {
    if(index==BeltGrenade) return GRENADE_SLOT;
    const u16 slots[]={KNIFE_SLOT,BINOCULAR_SLOT,DETECTOR_SLOT,BOLT_SLOT};
    return index>=0&&index<4?slots[index]:NO_ACTIVE_SLOT;
}
struct BeltHand { bool armed=false,held=false,returning=false,overPort=false; int index=-1; u16 item=u16(-1),weapon=u16(-1); Fmatrix pose,reloadOffset;shared_str reloadProfile;bool reloadLauncher=false; };
struct BodyEquipmentState {
    bool entered=false;
    u32 chestPress[2]{},backPress[2]{},templePress[2]{};
    bool templePending[2]{},templeLong[2]{};
    int primeGrenadeHand=-1;
    BeltHand hands[2]; int hover[2]{-1,-1};
    u32 nextAction[2]{},nextHoverPulse[2]{}; u16 pendingSlot=NO_ACTIVE_SLOT; int pendingHand=1; u32 pendingUntil=0;
    u16 extract=u16(-1); int extractHand=1;
};
inline BodyEquipmentState& bodyEquipmentState() { static BodyEquipmentState value; return value; }
inline void resetBodyEquipment() { bodyEquipmentState()=BodyEquipmentState{}; }
inline CInventoryItem* beltItem(CActor& actor,int index) {
    if(index<0||index>=BeltCount) return nullptr;
    if(index<4||index==BeltGrenade) return actor.inventory().ItemFromSlot(beltNativeSlot(index));
    if(index==BeltAmmo) {
        auto weapon=smart_cast<CWeapon*>(actor.inventory().ActiveItem());
        if(!weapon||weapon->IsPending()||weapon->m_ammoTypes.empty()) return nullptr;
        const auto type=weapon->m_set_next_ammoType_on_reload<weapon->m_ammoTypes.size()?weapon->m_set_next_ammoType_on_reload:weapon->GetAmmoType();
        if(type>=weapon->m_ammoTypes.size()) return nullptr;
        auto item=actor.inventory().GetAny(weapon->m_ammoTypes[type].c_str());
        auto ammo=smart_cast<CWeaponAmmo*>(item); return ammo&&ammo->m_boxCurr?item:nullptr;
    }
    const auto name=g_quick_use_slots[index-BeltQuick0];
    return name[0]?actor.inventory().GetAny(name):nullptr;
}
inline bool beltHandBusy(int side) { return bodyEquipmentState().hands[side].index==BeltAmmo; }
inline bool ammoProxyAtWeapon(CActor& actor,const Fvector& grip,u16 id) {
    auto weapon=smart_cast<CWeapon*>(actor.inventory().ActiveItem());
    auto hud=g_player_hud?g_player_hud->attached_item(0):nullptr;
    if(!weapon||weapon->ID()!=id||weapon->IsPending()||!hud||hud->m_parent_hud_item!=weapon||!gripProfile().valid) return false;
    Fvector local,port;
    if(auto gl=smart_cast<CWeaponMagazinedWGrenade*>(weapon);gl&&launcherProfile(weapon).size()) {
        if(gl->GetAmmoElapsed()!=0||!launcherPort(weapon,local,port))return false;
        interactionZones()[secondaryHand()].reload=true;interactionZones()[secondaryHand()].port=port;
        return grip.distance_to(port)<.075f;
    }
    if(auto gun=weapon->cast_weapon_magazined())if(gun->VRInternalKind()) {
        int chamber;return internalRoundAtWeapon(actor,grip,id,chamber);
    }
    if(auto gun=weapon->cast_weapon_magazined()) if(gun->VRPhysicalMagazine()) {
        if(gun->VRMagazinePresent()||!magazinePort(weapon,local,port)) return false;
        interactionZones()[secondaryHand()].reload=true; interactionZones()[secondaryHand()].port=port;
        return grip.distance_to(port)<.065f;
    }
    local=gripProfile().right; Fvector offset; offset.set(0,-.07f,.05f);
    if(pSettings->line_exist(weapon->cNameSect(),"copvr_reload_offset")) offset=pSettings->r_fvector3(weapon->cNameSect(),"copvr_reload_offset");
    local.add(offset); hud->m_item_transform.transform_tiny(port,local);
    interactionZones()[secondaryHand()].reload=true; interactionZones()[secondaryHand()].port=port;
    return grip.distance_to(port)<copvr::reloadRadius;
}
inline bool updateBodyEquipment(CActor& actor,const CopVrControls& controls,const Fmatrix& camera,bool allowed,bool secondaryActive) {
    auto& state=bodyEquipmentState(); Fmatrix body,chest;
    if(!allowed||!bodyFrame(camera,body)||!chestFrame(camera,chest)) {
        for(int side=0;side<2;++side) {
            const auto& h=state.hands[side];
            if(h.index==BeltAmmo) { occupiedHands()[side]=false; heldObjectRadius()[side]=0; }
            if((h.index>=0&&h.index<4)||h.index==BeltGrenade) {
                if(h.index==BeltDetector) { if(auto detector=smart_cast<CCustomDetector*>(actor.inventory().ItemFromSlot(DETECTOR_SLOT))) detector->HideDetector(true); }
                else if(actor.inventory().GetActiveSlot()==beltNativeSlot(h.index)) actor.inventory().Activate(NO_ACTIVE_SLOT);
            }
        }
        resetBodyEquipment(); return false;
    }
    bool changed=false;
    if(!state.entered) {
        state.entered=true;
        const auto slot=actor.inventory().GetActiveSlot();
        if((slot==INV_SLOT_2||slot==INV_SLOT_3)&&controls.hands[primaryHand()].squeeze<.25f) {
            actor.inventory().Action(kWPN_FIRE,CMD_STOP); actor.inventory().Activate(NO_ACTIVE_SLOT);
            changed=true;
        }
    }
    if(state.pendingSlot!=NO_ACTIVE_SLOT) {
        if(actor.inventory().GetActiveSlot()==state.pendingSlot) {
            primaryHand()=state.pendingHand; resetSupportGrip(); changed=true;
            pulse(state.pendingHand,.25f,30); state.pendingSlot=NO_ACTIVE_SLOT;
        } else if(Device.dwTimeGlobal>state.pendingUntil||actor.inventory().GetNextActiveSlot()!=state.pendingSlot) {
            if(state.pendingSlot==GRENADE_SLOT) {
                state.primeGrenadeHand=-1;
                if(state.hands[state.pendingHand].index==BeltGrenade) state.hands[state.pendingHand].index=-1;
            }
            state.pendingSlot=NO_ACTIVE_SLOT;
        }
    }
    Fmatrix inverse,chestInverse; inverse.invert(body);chestInverse.invert(chest);
    for(int side=0;side<2;++side) {
        auto& h=state.hands[side]; Fmatrix aim; Fvector grip,local,chestLocal;
        if(!hand(controls,side,camera,aim,grip)) { state.templePending[side]=false; if(h.index==BeltAmmo) { occupiedHands()[side]=false;heldObjectRadius()[side]=0; } h=BeltHand{}; continue; }
        grip=trackedPalm(aim,grip,side);
        inverse.transform_tiny(local,grip);chestInverse.transform_tiny(chestLocal,grip); aim.c=grip;
        const float squeeze=controls.hands[side].squeeze;
        const bool released=h.held&&squeeze<.25f;
        bool pressed=h.armed&&squeeze>.7f;
        if(squeeze<.25f) h.armed=true;
        if(pressed) { h.armed=false; h.held=true; }
        if(released) { h.held=false; state.chestPress[side]=state.backPress[side]=0; }
        const bool templeAvailable=torchTempleZone(camera,grip)&&actor.inventory().ItemFromSlot(TORCH_SLOT)&&
            !(actor.inventory().ActiveItem()&&side==primaryHand())&&!(secondaryActive&&side==secondaryHand())&&
            !occupiedHands()[side]&&!claimedHands()[side]&&h.index!=BeltAmmo&&state.pendingSlot==NO_ACTIVE_SLOT;
        if(!templeAvailable)state.templePending[side]=false;
        if(h.index==BeltAmmo) {
            auto active=smart_cast<CWeapon*>(actor.inventory().ActiveItem());
            if(!active||active->ID()!=h.weapon||h.reloadLauncher!=(launcherProfile(active).size()!=0)) {
                h.index=-1;h.item=h.weapon=u16(-1);h.reloadProfile="";occupiedHands()[side]=false;heldObjectRadius()[side]=0;continue;
            }
            h.pose.mul_43(aim,h.reloadOffset);
            claimedHands()[side]=true; occupiedHands()[side]=true; heldObjectRadius()[side]=.05f;
            if(auto active=smart_cast<CWeaponMagazined*>(actor.inventory().ActiveItem()))
                if(launcherProfile(active).size())heldObjectRadius()[side]=.025f;
                else if(active->VRInternalKind())heldObjectRadius()[side]=active->VRInternalKind()==3?.055f:.012f;
            const bool over=ammoProxyAtWeapon(actor,grip,h.weapon);
            if(over&&!h.overPort) pulse(side,.15f,20);
            h.overPort=over;
            auto gun=smart_cast<CWeaponMagazined*>(actor.inventory().ActiveItem());
            const bool physical=gun&&gun->ID()==h.weapon&&gun->VRPhysicalMagazine();
            bool inserted=false;
            // A fresh magazine seats on contact, without waiting for release.
            if(physical&&over&&squeeze>.25f) {
                actor.inventory().Action(kWPN_FIRE,CMD_STOP);
                inserted=gun->VRInsertMagazine();
                if(inserted) { pulse(side,.45f,45); changed=true; Msg("* CoPVR: magazine inserted weapon=%u; manual rack=%d",gun->ID(),gun->VRRequiresManualRack()?1:0); }
            }
            if(released||inserted) {
                if(over&&!physical) {
                    if(auto gl=smart_cast<CWeaponMagazinedWGrenade*>(gun);gl&&gl->ID()==h.weapon&&launcherProfile(gl).size()) {
                        auto ammo=smart_cast<CWeaponAmmo*>(Level().Objects.net_Find(h.item));
                        if(gl->VRLoadGrenadeRound(ammo)){pulse(side,.45f,45);changed=true;}
                    } else if(gun&&gun->ID()==h.weapon&&gun->VRInternalKind()) {
                        int chamber;if(internalRoundAtWeapon(actor,grip,h.weapon,chamber)) {
                            auto ammo=smart_cast<CWeaponAmmo*>(Level().Objects.net_Find(h.item));
                            if(gun->VRLoadInternalRound(ammo,chamber)){pulse(side,.35f,30);changed=true;}
                        }
                    } else {
                        actor.inventory().Action(kWPN_FIRE,CMD_STOP);
                        actor.inventory().Action(kWPN_RELOAD,CMD_START);pulse(side,.25f,25);changed=true;
                    }
                }
                h.index=-1; h.item=h.weapon=u16(-1); h.overPort=false; occupiedHands()[side]=false; heldObjectRadius()[side]=0.f;
            }
            continue;
        }
        if(h.index>=0&&h.index<4&&h.index!=BeltBolt) {
            if(released) h.returning=true;
            if(h.returning&&state.pendingSlot!=beltNativeSlot(h.index)) {
                auto item=actor.inventory().ItemFromSlot(beltNativeSlot(h.index));
                auto hud=item?item->cast_hud_item():nullptr;
                if(!hud||!hud->IsPending()) {
                    if(h.index==BeltDetector) { if(auto detector=smart_cast<CCustomDetector*>(item)) detector->HideDetector(true); }
                    else if(actor.inventory().GetActiveSlot()==beltNativeSlot(h.index)) actor.inventory().Activate(NO_ACTIVE_SLOT);
                    h.index=-1; h.returning=false; pulse(side,.15f,20); changed=true;
                }
            }
        }
        // A held item passing through another anchor can never replace it.
        const bool main=actor.inventory().ActiveItem()&&side==primaryHand();
        const bool secondary=secondaryActive&&side==secondaryHand();
        if(main&&released) {
            auto active=actor.inventory().ActiveItem()->cast_hud_item();
            const u16 slot=actor.inventory().GetActiveSlot();
            if((slot==INV_SLOT_2||slot==INV_SLOT_3)&&active&&!active->IsPending()&&
                (headWeaponZone(camera,grip,true)||inventoryStowZone(camera,grip))) {
                actor.inventory().Action(kWPN_FIRE,CMD_STOP); actor.inventory().Activate(NO_ACTIVE_SLOT);
                resetSupportGrip(); pulse(side,.25f,25); changed=true;
            }
        }
        if(main) {
            const u16 ownSlot=actor.inventory().GetActiveSlot();
            if(ownSlot==INV_SLOT_2||ownSlot==INV_SLOT_3) {
                interactionZones()[side].holster=true;
                interactionZones()[side].holsterPoint=grip;
            }
            const int target=(ownSlot==INV_SLOT_2||ownSlot==INV_SLOT_3)&&headWeaponZone(camera,grip,true)?BeltCount+ownSlot:-1;
            if(target>=0&&state.hover[side]!=target) pulse(side,.18f,25);
            state.hover[side]=target; continue;
        }
        if(secondary||occupiedHands()[side]||claimedHands()[side]||h.index==BeltAmmo||state.pendingSlot!=NO_ACTIVE_SLOT) { state.hover[side]=-1; continue; }
        // Close side-of-head gesture takes priority over the broad rear rack.
        if(templeAvailable) {
            if(pressed&&Device.dwTimeGlobal>=state.nextAction[side]) {
                state.templePress[side]=Device.dwTimeGlobal;
                state.templePending[side]=true;state.templeLong[side]=false;
            }
            if(state.templePending[side]) {
                if(!state.templeLong[side]&&squeeze>=.25f&&Device.dwTimeGlobal-state.templePress[side]>=650) {
                    actor.SwitchNightVision();state.templeLong[side]=true;
                    pulse(side,.65f,65);changed=true;
                }
                if(released) {
                    if(!state.templeLong[side]) {actor.SwitchTorch();pulse(side,.35f,40);changed=true;}
                    state.templePending[side]=false;state.nextAction[side]=Device.dwTimeGlobal+250;
                }
            }
            state.hover[side]=-1;continue;
        }
        // Rear waist: plain grip keeps knife/bolt; grip+index trigger selects
        // a reload activator for the firearm held by the opposite hand.
        auto reloadWeapon=smart_cast<CWeapon*>(actor.inventory().ActiveItem());
        const bool reloadAvailable=side==secondaryHand()&&reloadWeapon&&!detachedMagazines()[side].held&&backToolsZoneLocal(local)&&beltItem(actor,BeltAmmo);
        if(pressed&&reloadAvailable&&controls.hands[side].trigger<.75f) {
            state.backPress[side]=Device.dwTimeGlobal+90;
            claimedHands()[side]=true; continue;
        }
        if(state.backPress[side]) {
            if(squeeze<.25f||!reloadAvailable) state.backPress[side]=0;
            else if(controls.hands[side].trigger>.75f||Device.dwTimeGlobal>=state.backPress[side]) {
                state.backPress[side]=0; pressed=true;
            } else { claimedHands()[side]=true; continue; }
        }
        const bool reloadChord=pressed&&reloadAvailable&&controls.hands[side].trigger>.75f;
        if(reloadChord) claimedHands()[side]=true;
        // A short chord window tolerates grip/trigger arriving in adjacent frames.
        if(pressed&&chestZoneLocal(chestLocal)&&controls.hands[side].trigger<.75f) {
            state.chestPress[side]=Device.dwTimeGlobal+90;
            claimedHands()[side]=true; continue;
        }
        if(state.chestPress[side]) {
            if(squeeze<.25f||!chestZoneLocal(chestLocal)) state.chestPress[side]=0;
            else if(controls.hands[side].trigger>.75f||Device.dwTimeGlobal>=state.chestPress[side]) {
                state.chestPress[side]=0; pressed=true;
            } else { claimedHands()[side]=true; continue; }
        }
        const bool grenadeChord=pressed&&chestZoneLocal(chestLocal)&&controls.hands[side].trigger>.75f;
        if(grenadeChord) claimedHands()[side]=true; // consume the chord, do not also send F/use

        int selected=-1; float best=1.f;
        bool available[BeltCount]{};
        // A shared chest region selects equipment by hand, not by tiny targets.
        const int chestIndex=side==0?BeltDetector:BeltBinocular;
        auto chestItem=beltItem(actor,chestIndex);
        if(chestZoneLocal(chestLocal)&&chestItem&&
            chestItem!=actor.inventory().ActiveItem()&&!isExtraWeapon(smart_cast<CWeapon*>(chestItem))) {
            selected=chestIndex; best=0; available[chestIndex]=true;
        }
        const int quick=copvr::quickZone(local.x,local.y,local.z,side);
        if(quick>=0&&beltItem(actor,BeltQuick0+quick)) {
            selected=BeltQuick0+quick; best=0; available[selected]=true;
        }
        if(grenadeChord&&beltItem(actor,BeltGrenade)) {
            selected=BeltGrenade; best=0; available[BeltGrenade]=true;
        }
        const bool firearmActive=reloadWeapon&&(actor.inventory().GetActiveSlot()==INV_SLOT_2||actor.inventory().GetActiveSlot()==INV_SLOT_3);
        const int backIndex=firearmActive&&side!=primaryHand()?BeltBolt:side==0?BeltKnife:BeltBolt;
        auto backItem=beltItem(actor,backIndex);
        const bool knifeBlocked=backIndex==BeltKnife&&primaryHand()==1&&reloadWeapon&&
            (actor.inventory().GetActiveSlot()==INV_SLOT_2||actor.inventory().GetActiveSlot()==INV_SLOT_3);
        if(!knifeBlocked&&backToolsZoneLocal(local)&&backItem&&backItem!=actor.inventory().ActiveItem()) {
            bool inHand=false; for(const auto& other:state.hands) if(other.index==backIndex) inHand=true;
            if(!inHand) { selected=backIndex; best=0; available[backIndex]=true; }
        }
        if(reloadChord) { selected=BeltAmmo; best=0; available[BeltAmmo]=true; }
        u16 weaponSlot=headWeaponSlot(actor,camera,grip,side);
        if(weaponSlot!=NO_ACTIVE_SLOT) {
            best=0; selected=-1;
        }
        int hover=selected>=0?selected:weaponSlot!=NO_ACTIVE_SLOT?BeltCount+weaponSlot:-1;
        const int previous=state.hover[side];
        if(!grenadeChord&&!reloadChord&&weaponSlot==NO_ACTIVE_SLOT&&previous>=0&&previous<BeltCount&&available[previous]) {
            const float distance=previous==selected&&best==0?0.f:(previous==BeltAmmo?beltReach(local,previous):2.f);
            if(distance<1.12f&&distance<=best+.12f) { selected=previous; weaponSlot=NO_ACTIVE_SLOT; hover=previous; }
        }
        if(hover!=previous) {
            state.hover[side]=hover;
            if(hover>=0&&Device.dwTimeGlobal>=state.nextHoverPulse[side]) {
                pulse(side,.22f,35); state.nextHoverPulse[side]=Device.dwTimeGlobal+140;
            }
        }
        if(!pressed||Device.dwTimeGlobal<state.nextAction[side]) continue;
        if(selected>=BeltQuick0&&selected<=BeltQuick3) {
            auto item=beltItem(actor,selected);
            state.extract=item->object().ID(); state.extractHand=side;
            state.nextAction[side]=Device.dwTimeGlobal+150; changed=true;
        } else if(selected==BeltAmmo) {
            auto weapon=smart_cast<CWeapon*>(actor.inventory().ActiveItem()); auto item=beltItem(actor,selected);
            if(weapon&&item) { h.index=selected; h.item=item->object().ID(); h.weapon=weapon->ID(); h.reloadLauncher=launcherProfile(weapon).size()!=0;h.reloadProfile=magazineProfile(weapon);
                if(auto gun=weapon->cast_weapon_magazined();gun&&gun->VRInternalKind())h.reloadProfile=equipmentConfig()->r_string(gun->VRInternalProfile(),"round_profile");
                if(h.reloadLauncher)h.reloadProfile=equipmentConfig()->r_string(launcherProfile(weapon),"round_profile");
                h.reloadOffset=reloadObjectOffset(weapon,side); h.pose.mul_43(aim,h.reloadOffset); claimedHands()[side]=true; occupiedHands()[side]=true; heldObjectRadius()[side]=.05f; pulse(side,.2f,20); }
        } else if(selected==BeltBolt&&reloadWeapon&&side!=primaryHand()&&
            (actor.inventory().GetActiveSlot()==INV_SLOT_2||actor.inventory().GetActiveSlot()==INV_SLOT_3)) {
            if(beginAuxBolt(actor,side,aim)){changed=true;state.nextAction[side]=Device.dwTimeGlobal+150;}
        } else if(selected>=0) {
            h.index=selected; h.returning=false;
            if(selected==BeltDetector) {
                if(!actor.inventory().ActiveItem()) primaryHand()=1-side;
                auto detector=smart_cast<CCustomDetector*>(beltItem(actor,selected));
                if(detector) detector->ToggleDetector(actor.inventory().ActiveItem()!=nullptr);
                pulse(side,.2f,20); changed=true;
            } else {
                const u16 slot=beltNativeSlot(selected);
                state.primeGrenadeHand=selected==BeltGrenade&&grenadeChord?side:-1;
                actor.inventory().Activate(slot); state.pendingSlot=slot; state.pendingHand=side; state.pendingUntil=Device.dwTimeGlobal+2500; changed=true;
            }
            state.nextAction[side]=Device.dwTimeGlobal+150;
        } else if(weaponSlot!=NO_ACTIVE_SLOT) {
            state.primeGrenadeHand=-1;
            actor.inventory().Activate(weaponSlot); state.pendingSlot=weaponSlot; state.pendingHand=side; state.pendingUntil=Device.dwTimeGlobal+2500;
            h.index=-1; state.nextAction[side]=Device.dwTimeGlobal+150; changed=true;
        }
    }
    return changed;
}
}
