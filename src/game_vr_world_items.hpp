#pragma once
#include "game_vr_controls.hpp"
#include "game_vr_body.hpp"
#include "game_vr_motion.hpp"
#include "xrGame/Actor.h"
#include "xrGame/Inventory.h"
#include "xrGame/inventory_item.h"
#include "xrGame/PhysicsShellHolder.h"
#include "xrGame/Level.h"
#include "xrGame/GamePersistent.h"
#include "xrGame/game_cl_base.h"
#include "xrGame/ExplosiveRocket.h"
#include "xrGame/Missile.h"
#include "xrGame/Grenade.h"
#include "xrGame/eatable_item.h"
#include "xrGame/WeaponAmmo.h"
#include "xrGame/player_hud.h"
#include "xrPhysics/PhysicsShell.h"
#include "xrServerEntities/xrMessages.h"
#include "game_vr_collision.hpp"

namespace copvr_game {
enum class WorldIntent { Stow,Consume,Equip,EquipGrenade };
struct WorldGrip {
    u16 id=u16(-1);
    bool armed=false,triggerArmed=false;
    u16 pending=u16(-1);
    u32 pendingUntil=0;
    WorldIntent intent=WorldIntent::Stow;
    u32 useSince=0;
    bool stowHover=false;
    Fmatrix offset;
    MotionHistory motion;
};
inline WorldGrip* worldGrips() { static WorldGrip value[2]; return value; }
struct InventoryExtraction { u16 item=u16(-1),actor=u16(-1); int side=1; u32 until=0; };
inline InventoryExtraction& inventoryExtraction() { static InventoryExtraction value; return value; }
struct OwnedRelease {
    u16 item=u16(-1),actor=u16(-1); u32 until=0;
    Fmatrix pose; Fvector linear,angular;
};
inline OwnedRelease* ownedReleases() { static OwnedRelease values[2]; return values; }
inline bool ownedReleaseFor(u16 id) { for(int i=0;i<2;++i) if(ownedReleases()[i].item==id) return true; return false; }
inline bool queueOwnedRelease(CActor& actor,CInventoryItem& item,const Fmatrix& pose,const Fvector& linear,const Fvector& angular) {
    OwnedRelease* free=nullptr; for(int i=0;i<2;++i) if(ownedReleases()[i].item==u16(-1)) { free=&ownedReleases()[i]; break; }
    if(!free||ownedReleaseFor(item.object().ID())||item.object().H_Parent()!=&actor||item.IsQuestItem()||!item.CanTake()) return false;
    auto& release=*free;
    if(actor.inventory().InSlot(&item)&&actor.inventory().SlotIsPersistent(item.CurrSlot())) return false;
    release.item=item.object().ID(); release.actor=actor.ID(); release.until=Device.dwTimeGlobal+2000;
    release.pose=pose; release.linear=linear; release.angular=angular;
    if(auto weapon=smart_cast<CWeapon*>(&item)) weapon->Action(kWPN_FIRE,CMD_STOP);
    resetSupportGrip();
    NET_Packet packet; item.object().u_EventGen(packet,GE_OWNERSHIP_REJECT,actor.ID());
    packet.w_u16(item.object().ID()); item.object().u_EventSend(packet);
    return true;
}
inline bool queueInventoryExtraction(CActor& actor,CInventoryItem& item,int side) {
    auto& request=inventoryExtraction();
    if(side<0||side>1||request.item!=u16(-1)||item.object().H_Parent()!=&actor||item.IsQuestItem()||
        !item.CanTake()||occupiedHands()[side]||extraHand(side)) return false;
    if(actor.inventory().InSlot(&item)&&actor.inventory().SlotIsPersistent(item.CurrSlot())) return false;
    auto grenade=smart_cast<CGrenade*>(&item);
    if((smart_cast<CMissile*>(&item)&&!(grenade&&grenade->Useful()))||smart_cast<CExplosiveRocket*>(&item)) return false;
    if(side==primaryHand()&&actor.inventory().ActiveItem()) {
        actor.inventory().Action(kWPN_FIRE,CMD_STOP); actor.inventory().Activate(NO_ACTIVE_SLOT);
    }
    request.item=item.object().ID(); request.actor=actor.ID(); request.side=side;
    request.until=Device.dwTimeGlobal+2500;
    resetSupportGrip(); return true;
}
inline CInventoryItem* worldItem(u16 id) {
    if(id==u16(-1)) return nullptr;
    auto item=smart_cast<CInventoryItem*>(Level().Objects.net_Find(id));
    return item&&!item->object().getDestroy()&&!item->object().H_Parent()?item:nullptr;
}
inline void releaseWorldItem(int side,bool throwing) {
    auto& held=worldGrips()[side];
    if(auto item=worldItem(held.id)) {
        if(auto actor=smart_cast<CActor*>(Level().CurrentControlEntity())) guardReleasedCollision(*actor,*item);
        if(auto shell=item->object().PPhysicsShell()) {
            shell->EnableCollision(); shell->Enable();
            Fvector velocity; velocity.set(0,0,0);
            if(throwing) velocity=held.motion.throwLinear();
            shell->set_LinearVel(velocity);
            shell->set_AngularVel(throwing?held.motion.angular:Fvector().set(0,0,0));
        }
    }
    held.id=u16(-1); held.armed=false; held.motion.reset(); occupiedHands()[side]=extraHand(side);
    heldObjectRadius()[side]=extraHand(side)?.04f:0.f;
    held.useSince=0; held.stowHover=false;
}
inline void releaseWorldItems(bool resetWeapon=true) {
    releaseWorldItem(0,false); releaseWorldItem(1,false);
    resetReleasedCollisions();
    worldGrips()[0].pending=worldGrips()[1].pending=u16(-1);
    inventoryExtraction()=InventoryExtraction{};
    ownedReleases()[0]=ownedReleases()[1]=OwnedRelease{};
    if(resetWeapon) primaryHand()=1;
    weaponTransferLatched()=false;
    resetSupportGrip();
}
inline bool reachableItem(CActor& actor,CInventoryItem& item,const Fvector& grip) {
    Fvector center,direction; item.object().Center(center); direction.sub(center,grip);
    const float distance=direction.magnitude();
    if(distance<.001f) return true;
    direction.div(distance); collide::rq_result hit;
    return !Level().ObjectSpace.RayPick(grip,direction,distance,collide::rqtBoth,hit,&actor)||hit.O==&item.object();
}
inline bool worldWeaponHandle(CInventoryItem& item,const Fvector& palm) {
    auto weapon=smart_cast<CWeapon*>(&item);
    if(!weapon||weapon->BaseSlot()==KNIFE_SLOT||weapon->BaseSlot()==BINOCULAR_SLOT) return false;
    const auto section=item.object().cNameSect(); Fvector local;
    if(pSettings->line_exist(section,"copvr_world_grip")) local=pSettings->r_fvector3(section,"copvr_world_grip");
    else {
        // Use the game's authored world-model hand attachment. The inverse
        // maps the actor's hand origin into the weapon, without a bbox guess.
        Fmatrix attachment,inverse;
        auto rotation=pSettings->r_fvector3(section,"orientation"); rotation.mul(PI/180.f);
        attachment.setHPB(rotation.x,rotation.y,rotation.z); attachment.c=pSettings->r_fvector3(section,"position");
        inverse.invert(attachment); local=inverse.c;
    }
    Fvector world; item.object().XFORM().transform_tiny(world,local);
    return palm.distance_to(world)<=.10f;
}
inline bool equipWorldWeapon(CActor& actor,CInventoryItem& item,int side) {
    auto& inventory=actor.inventory();
    if(!smart_cast<CWeapon*>(&item)||item.object().H_Parent()!=&actor||inventory.IsSlotBlocked(&item)) return false;
    u16 slot=inventory.InSlot(&item)?item.CurrSlot():NO_ACTIVE_SLOT;
    if(slot==NO_ACTIVE_SLOT) {
        // Prefer an empty compatible slot for an explicit handle grip.
        const u16 candidates[]={item.BaseSlot(),INV_SLOT_2,INV_SLOT_3};
        for(auto candidate:candidates) if(candidate!=NO_ACTIVE_SLOT&&candidate<=inventory.LastSlot()&&
            inventory.CanPutInSlot(&item,candidate)&&inventory.Slot(candidate,&item,true)) { slot=candidate; break; }
    }
    if(slot==NO_ACTIVE_SLOT) {
        // A grip on a third weapon is an explicit equip request. Make room
        // in an inactive weapon slot by storing its occupant, never dropping it.
        const u16 candidates[]={u16(side==0?INV_SLOT_2:INV_SLOT_3),u16(side==0?INV_SLOT_3:INV_SLOT_2)};
        for(auto candidate:candidates) {
            if(candidate==inventory.GetActiveSlot()||inventory.SlotIsPersistent(candidate)) continue;
            auto previous=inventory.ItemFromSlot(candidate);
            if(!previous||inventory.IsSlotBlocked(previous)||!inventory.CanPutInRuck(previous)) continue;
            if(!inventory.Ruck(previous)) continue;
            if(inventory.CanPutInSlot(&item,candidate)&&inventory.Slot(candidate,&item,true)) { slot=candidate; break; }
            inventory.Slot(candidate,previous,true); // roll back incompatible placement
        }
    }
    if(slot==NO_ACTIVE_SLOT) return false;
    inventory.Action(kWPN_FIRE,CMD_STOP); primaryHand()=side; resetSupportGrip();
    inventory.Activate(slot); pulse(side,.3f,30);
    Msg("* CoPVR: world weapon equipped hand=%d id=%u slot=%u",side,item.object().ID(),slot);
    return true;
}
inline bool storeWorldItem(CActor& actor,int side,WorldIntent intent) {
    auto& held=worldGrips()[side]; auto item=worldItem(held.id);
    if(!item||!actor.inventory().CanTakeItem(item)) return false;
    const u16 id=held.id;
    releaseWorldItem(side,false);
    if(intent!=WorldIntent::Stow) {
        held.pending=id; held.pendingUntil=Device.dwTimeGlobal+2000; held.intent=intent;
        claimedHands()[side]=true;
    }
    Game().SendPickUpEvent(actor.ID(),id); pulse(side,.2f,20);
    Msg("* CoPVR: hand %d stow request for world item %u",side,id);
    return true;
}
inline bool equipInventoryGrenade(CActor& actor,CInventoryItem& item,int side) {
    auto grenade=smart_cast<CGrenade*>(&item); auto& inventory=actor.inventory();
    if(!grenade||!grenade->Useful()||item.object().H_Parent()!=&actor||inventory.IsSlotBlocked(&item)) return false;
    auto previous=inventory.ItemFromSlot(GRENADE_SLOT);
    if(previous!=&item) {
        if(previous&&(inventory.GetActiveSlot()==GRENADE_SLOT||!inventory.Ruck(previous))) return false;
        if(!inventory.Slot(GRENADE_SLOT,&item,true)) {
            if(previous) inventory.Slot(GRENADE_SLOT,previous,true);
            return false;
        }
    }
    primaryHand()=side; resetSupportGrip(); inventory.Activate(GRENADE_SLOT); pulse(side,.3f,30); return true;
}
inline void updateWorldItems(CActor& actor,const CopVrControls& controls,const Fmatrix& camera,bool allowed,bool weaponActive,bool secondaryActive) {
    interactionZones()[0]=interactionZones()[1]=InteractionZones{};
    updateReleasedCollisions(worldGrips()[0].id,worldGrips()[1].id);
    auto held=worldGrips();
    for(int releaseIndex=0;releaseIndex<2;++releaseIndex) {
    auto& release=ownedReleases()[releaseIndex];
    if(release.item!=u16(-1)) {
        auto item=worldItem(release.item);
        if(release.actor!=actor.ID()||Device.dwTimeGlobal>release.until) release=OwnedRelease{};
        else if(item&&item->object().PPhysicsShell()&&item->object().PPhysicsShell()->isActive()) {
            guardReleasedCollision(actor,*item);
            auto& object=item->object(); auto shell=object.PPhysicsShell();
            shell->SetGlTransformDynamic(release.pose); object.XFORM()=release.pose; object.spatial_move(); shell->Enable();
            Fvector zero; zero.set(0,0,0);
            shell->set_LinearVel(allowed?release.linear:zero); shell->set_AngularVel(allowed?release.angular:zero);
            Msg("* CoPVR: owned weapon released id=%u",object.ID()); release=OwnedRelease{};
        }
    }
    }
    auto& extraction=inventoryExtraction();
    if(extraction.item!=u16(-1)) {
        auto item=smart_cast<CInventoryItem*>(Level().Objects.net_Find(extraction.item));
        const int side=extraction.side;
        Fmatrix pose; Fvector grip;
        if(extraction.actor!=actor.ID()||Device.dwTimeGlobal>extraction.until||!item||item->object().getDestroy())
            extraction=InventoryExtraction{};
        else if(allowed&&hand(controls,side,camera,pose,grip)) {
            const bool free=!(weaponActive&&side==primaryHand())&&!(secondaryActive&&side==secondaryHand());
            if(controls.hands[side].squeeze<.25f) extraction=InventoryExtraction{};
            else if(free&&!item->object().H_Parent()&&item->object().PPhysicsShell()&&item->object().PPhysicsShell()->isActive()) {
                auto& object=item->object();
                Fmatrix inverse; inverse.invert(object.XFORM()); Fvector center,localCenter;
                object.Center(center); inverse.transform_tiny(localCenter,center);
                held[side].offset.identity();
                Fvector anchor; anchor.set(0,0,0);
                const auto& palm=palmSockets()[side]; if(palm.valid) anchor=palm.aimOffset;
                const auto& box=object.Visual()->getVisData().box;
                if(smart_cast<CWeapon*>(item)) {
                    localCenter.set((box.vMin.x+box.vMax.x)*.5f,
                        box.vMin.y+(box.vMax.y-box.vMin.y)*.25f,
                        box.vMin.z+(box.vMax.z-box.vMin.z)*.28f);
                    if(pSettings->line_exist(object.cNameSect(),"copvr_world_grip"))
                        localCenter=pSettings->r_fvector3(object.cNameSect(),"copvr_world_grip");
                }
                else if(palm.valid) anchor.mad(palm.normal,(box.vMax.x-box.vMin.x)*.5f+.012f);
                held[side].offset.c.sub(anchor,localCenter);
                held[side].id=object.ID(); held[side].armed=false;
                held[side].triggerArmed=controls.hands[side].trigger<.25f;
                held[side].motion.reset(); occupiedHands()[side]=true;
                heldObjectRadius()[side]=object.Radius();
                guardReleasedCollision(actor,*item);
                object.PPhysicsShell()->DisableCollision(); object.PPhysicsShell()->Disable(); extraction=InventoryExtraction{};
                pulse(side,.25f,25); Msg("* CoPVR: inventory extraction attached hand=%d id=%u",side,object.ID());
                if(auto grenade=smart_cast<CGrenade*>(item)) if(grenade->Useful()) storeWorldItem(actor,side,WorldIntent::EquipGrenade);
            }
        }
    }
    for(int side=0;side<2;++side) if(held[side].pending!=u16(-1)) {
        claimedHands()[side]=true;
        auto item=smart_cast<CInventoryItem*>(Level().Objects.net_Find(held[side].pending));
        if(!allowed||Device.dwTimeGlobal>held[side].pendingUntil||!item||item->object().getDestroy()) {
            held[side].pending=u16(-1); continue;
        }
        if(item->object().H_Parent()==&actor) {
            // Native Eat performs condition effects, remaining portions, scripts and removal.
            if(held[side].intent==WorldIntent::Consume) actor.inventory().Eat(item);
            else if(held[side].intent==WorldIntent::EquipGrenade&&controls.hands[side].valid&&controls.hands[side].squeeze>=.25f&&
                !(secondaryActive&&side==secondaryHand())) equipInventoryGrenade(actor,*item,side);
            else if(held[side].intent==WorldIntent::Equip&&controls.hands[side].valid&&controls.hands[side].squeeze>=.25f&&
                !(secondaryActive&&side==secondaryHand())) {
                if(!equipWorldWeapon(actor,*item,side)) Msg("! CoPVR: world equip unavailable, item kept in inventory id=%u",item->object().ID());
            }
            held[side].pending=u16(-1);
        }
    }
    // Transfer the existing object; never create a second inventory/world copy.
    for(int owner=0;owner<2;++owner) {
        const int other=1-owner;
        if(!allowed||held[owner].id==u16(-1)||held[other].id!=u16(-1)||held[owner].pending!=u16(-1)||held[other].pending!=u16(-1)||!held[other].armed||
            controls.hands[other].squeeze<.7f||(weaponActive&&other==primaryHand())||(secondaryActive&&other==secondaryHand())) continue;
        Fmatrix a,b; Fvector p,q;
        auto item=worldItem(held[owner].id);
        if(!item||!hand(controls,owner,camera,a,p)||!hand(controls,other,camera,b,q)) continue;
        if(worldWeaponHandle(*item,trackedPalm(b,q,other))&&storeWorldItem(actor,owner,WorldIntent::Equip)) {
            held[other].pending=held[owner].pending; held[other].pendingUntil=held[owner].pendingUntil; held[other].intent=WorldIntent::Equip;
            claimedHands()[other]=true;
            held[owner].pending=u16(-1); held[other].armed=false; continue;
        }
        if(p.distance_to(q)>.09f) continue;
        b.c=q; Fmatrix inverse; inverse.invert(b);
        held[other].offset.mul_43(inverse,item->object().XFORM());
        held[other].id=held[owner].id; held[other].motion.reset(); held[other].armed=false;
        held[other].triggerArmed=controls.hands[other].trigger<.25f;
        held[owner].id=u16(-1); held[owner].armed=false;
        occupiedHands()[owner]=false; occupiedHands()[other]=true; pulse(other,.2f,20);
        heldObjectRadius()[other]=heldObjectRadius()[owner]; heldObjectRadius()[owner]=0.f;
    }
    for(int side=0;side<2;++side) {
        if(held[side].pending!=u16(-1)) continue;
        Fmatrix pose; Fvector grip;
        const bool reserved=(weaponActive&&side==primaryHand())||(secondaryActive&&side==secondaryHand())||beltHandBusy(side)||extraHand(side);
        const bool tracked=allowed&&!reserved&&hand(controls,side,camera,pose,grip);
        if(!tracked) { releaseWorldItem(side,false); continue; }
        pose.c=grip;
        const auto& input=controls.hands[side];
        if(input.squeeze<.25f) held[side].armed=true;
        if(input.trigger<.25f) held[side].triggerArmed=true;
        auto item=worldItem(held[side].id);
        if(held[side].id!=u16(-1)&&!item) releaseWorldItem(side,false);
        if(item) {
            guardReleasedCollision(actor,*item);
            auto shell=item->object().PPhysicsShell();
            if(!shell||!shell->isActive()) { releaseWorldItem(side,false); continue; }
            Fmatrix transform; transform.mul_43(pose,held[side].offset);
            held[side].motion.update(unsmoothedHandPose(transform,camera,side),Device.fTimeDelta);
            shell->DisableCollision(); shell->Disable();
            shell->SetGlTransformDynamic(transform);
            item->object().XFORM()=transform; item->object().spatial_move();
            auto food=smart_cast<CEatableItem*>(item);
            interactionZones()[side].stow=true;
            interactionZones()[side].use=false;
            const bool use=held[side].triggerArmed&&input.trigger>.75f&&food&&!food->Empty();
            if(use) {
                held[side].triggerArmed=false;
                if(storeWorldItem(actor,side,WorldIntent::Consume)) continue;
            }
            if(!food&&held[side].triggerArmed&&input.trigger>.75f) {
                held[side].triggerArmed=false;
                item->object().use(&actor); // native scripted activation, if supported
                continue; // callback may remove or transfer the item
            }
            const bool pocket=inventoryStowZone(camera,trackedPalm(pose,grip,side));
            if(pocket&&!held[side].stowHover) pulse(side,.18f,25);
            held[side].stowHover=pocket;
            if(worldItem(held[side].id)&&input.squeeze<.25f) {
                if(!pocket||!storeWorldItem(actor,side,WorldIntent::Stow)) releaseWorldItem(side,true);
            }
            continue;
        }
        const bool unavailable=(weaponActive&&side==primaryHand())||(secondaryActive&&side==secondaryHand())||
            (supportGrip().active()&&side==secondaryHand());
        if(unavailable||!held[side].armed||input.squeeze<.7f) continue;
        held[side].armed=false;
        xr_vector<ISpatial*> nearby;
        g_pGamePersistent->SpatialSpace.q_sphere(nearby,0,STYPE_COLLIDEABLE,grip,.22f);
        CInventoryItem* nearest=nullptr; float best=.11f;
        for(auto spatial:nearby) {
            auto candidate=smart_cast<CInventoryItem*>(spatial->dcast_GameObject());
            if(!candidate||!candidate->CanTake()) continue;
            auto& object=candidate->object();
            if(object.getDestroy()||object.H_Parent()||object.ID()==held[1-side].id) continue;
            // Active projectiles and grenades retain their normal gameplay behavior.
            auto grenade=smart_cast<CGrenade*>(&object);
            if((smart_cast<CMissile*>(&object)&&!(grenade&&grenade->Useful()))||smart_cast<CExplosiveRocket*>(&object)) continue;
            auto shell=object.PPhysicsShell();
            if(!candidate->IsQuestItem()&&(!shell||!shell->isActive())) continue;
            if(worldWeaponHandle(*candidate,trackedPalm(pose,grip,side))) {
                if(reachableItem(actor,*candidate,grip)) { nearest=candidate; best=-1.f; }
                continue;
            }
            Fvector center; object.Center(center);
            const float radius=_min(object.Radius(),.12f);
            const float distance=_max(0.f,grip.distance_to(center)-radius);
            if(distance>=best||!reachableItem(actor,*candidate,grip)) continue;
            nearest=candidate; best=distance;
        }
        if(!nearest) continue;
        auto& object=nearest->object();
        if(nearest->IsQuestItem()) {
            // Quest loot uses native ownership and quest callbacks, never a
            // temporary physics grab/drop that would bypass its protection.
            if(actor.inventory().CanTakeItem(nearest)) {
                Game().SendPickUpEvent(actor.ID(),object.ID());
                pulse(side,.4f,45);
                Msg("* CoPVR: quest item pickup requested hand=%d id=%u",side,object.ID());
            }
            continue;
        }
        Fmatrix inverse; inverse.invert(pose);
        held[side].offset.mul_43(inverse,object.XFORM());
        held[side].motion.reset(); held[side].motion.update(unsmoothedHandPose(object.XFORM(),camera,side),Device.fTimeDelta);
        held[side].id=object.ID(); held[side].triggerArmed=input.trigger<.25f;
        guardReleasedCollision(actor,*nearest);
        object.PPhysicsShell()->DisableCollision(); object.PPhysicsShell()->Disable(); occupiedHands()[side]=true;
        heldObjectRadius()[side]=object.Radius();
        if(auto grenade=smart_cast<CGrenade*>(nearest)) if(grenade->Useful()) {
            if(storeWorldItem(actor,side,WorldIntent::EquipGrenade)) continue;
        }
        if(worldWeaponHandle(*nearest,trackedPalm(pose,grip,side))) {
            if(storeWorldItem(actor,side,WorldIntent::Equip)) continue;
        }
        pulse(side,.25f,25); Msg("* CoPVR: hand %d grabbed world item %u",side,object.ID());
    }
}
}
