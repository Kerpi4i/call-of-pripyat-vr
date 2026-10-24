#pragma once
#include "xrPhysics/ExtendedGeom.h"
#include "xrPhysics/PhysicsShell.h"
#include "xrGame/Actor.h"
#include "xrGame/Level.h"
#include "game_vr_collision_state.hpp"
namespace copvr_game {
inline void releasedContact(bool& collide,bool,dContact& contact,SGameMtl*,SGameMtl*) {
    auto a=PHRetrieveGeomUserData(contact.geom.g1),b=PHRetrieveGeomUserData(contact.geom.g2);
    if(!a||!b||!a->ph_ref_object||!b->ph_ref_object) return;
    auto actor=smart_cast<CActor*>(a->ph_ref_object);
    auto other=smart_cast<CPhysicsShellHolder*>(b->ph_ref_object);
    if(!actor) { actor=smart_cast<CActor*>(b->ph_ref_object); other=smart_cast<CPhysicsShellHolder*>(a->ph_ref_object); }
    if(!actor||!other) return;
    for(int i=0;i<64;++i) {
        const auto& entry=releaseCollisions()[i];
        if(entry.actor==actor->ID()&&entry.item==other->ID()) { collide=false; return; }
    }
}
inline void guardReleasedCollision(CActor& actor,CPhysicsShellHolder& item) {
    auto shell=item.PPhysicsShell(); if(!shell) return;
    ReleaseCollision* free=nullptr;
    for(int i=0;i<64;++i) {
        auto& entry=releaseCollisions()[i];
        if(entry.item==item.ID()) { entry.after=Device.dwTimeGlobal+250; return; }
        if(entry.item==u16(-1)&&!free) free=&entry;
    }
    if(!free) return;
    free->item=item.ID(); free->actor=actor.ID(); free->after=Device.dwTimeGlobal+250;
    shell->add_ObjectContactCallback(releasedContact);
}
inline void guardReleasedCollision(CActor& actor,CInventoryItem& item) {
    auto object=smart_cast<CPhysicsShellHolder*>(&item.object());
    if(object) guardReleasedCollision(actor,*object);
}
inline void updateReleasedCollisions(u16 held0=u16(-1),u16 held1=u16(-1)) {
    for(int i=0;i<64;++i) {
        auto& entry=releaseCollisions()[i]; if(entry.item==u16(-1)) continue;
        if(entry.item==held0||entry.item==held1) continue;
        auto object=smart_cast<CPhysicsShellHolder*>(Level().Objects.net_Find(entry.item));
        auto actor=smart_cast<CActor*>(Level().Objects.net_Find(entry.actor));
        if(!object||object->getDestroy()) { entry=ReleaseCollision{}; continue; }
        Fvector center; object->Center(center);
        Fvector actorCenter; if(actor) actor->Center(actorCenter);
        if(!actor||object->H_Parent()||(Device.dwTimeGlobal>=entry.after&&
            center.distance_to(actorCenter)>actor->Radius()+object->Radius()+.10f)) {
            if(auto shell=object->PPhysicsShell()) shell->remove_ObjectContactCallback(releasedContact);
            entry=ReleaseCollision{};
        }
    }
}
inline void resetReleasedCollisions() {
    for(int i=0;i<64;++i) {
        auto& entry=releaseCollisions()[i];
        if(entry.item!=u16(-1)) if(auto object=smart_cast<CPhysicsShellHolder*>(Level().Objects.net_Find(entry.item)))
            if(auto shell=object->PPhysicsShell()) shell->remove_ObjectContactCallback(releasedContact);
        entry=ReleaseCollision{};
    }
}
}
