#pragma once
#include "game_vr_world_items.hpp"
#include "xrGame/entity_alive.h"
namespace copvr_game {
struct PhysicsGrip {
    u16 object=u16(-1),element=u16(-1);
    bool armed=false;
    Fvector local{},lastTarget{};
    bool targetValid=false;
    MotionHistory motion;
};
inline PhysicsGrip* physicsGrips() { static PhysicsGrip value[2]; return value; }
inline CPhysicsShellHolder* physicsGrabObject(u16 id);
inline void releasePhysicsGrip(int side,bool throwing=false) {
    auto& grip=physicsGrips()[side];
    if(grip.object!=u16(-1)) {
        if(throwing&&grip.motion.valid&&physicsGrips()[1-side].object!=grip.object) {
            auto object=physicsGrabObject(grip.object);
            if(object) {
                auto shell=object->PPhysicsShell(); Fvector desired=grip.motion.throwLinear();
                if(desired.square_magnitude()>.25f) {
                    Fvector velocity,delta; shell->get_LinearVel(velocity); delta.sub(desired,velocity);
                    const float speed=delta.magnitude(); if(speed>24.f) delta.mul(24.f/speed);
                    shell->Enable();
                    for(u16 i=0;i<shell->get_ElementsNumber();++i) {
                        auto element=shell->get_ElementByStoreOrder(i);
                        if(element&&!element->isFixed()) element->applyImpulse(delta,element->getMass());
                    }
                }
            }
        }

        occupiedHands()[side]=false; heldObjectRadius()[side]=0;
        Msg("* CoPVR: physics grip released hand=%d id=%u",side,grip.object);
    }
    grip=PhysicsGrip{}; // a new press is required after every interruption
}
inline void resetPhysicsGrips() { releasePhysicsGrip(0); releasePhysicsGrip(1); }
inline CPhysicsShellHolder* physicsGrabObject(u16 id) {
    auto object=id!=u16(-1)?smart_cast<CPhysicsShellHolder*>(Level().Objects.net_Find(id)):nullptr;
    if(!object||object->getDestroy()||object->H_Parent()||smart_cast<CInventoryItem*>(object)) return nullptr;
    auto alive=smart_cast<CEntityAlive*>(object);
    if(alive&&alive->g_Alive()) return nullptr;
    auto shell=object->PPhysicsShell();
    return shell&&shell->isActive()?object:nullptr;
}
inline bool physicsPointVisible(CActor& actor,CPhysicsShellHolder& object,const Fvector& palm,const Fvector& point) {
    Fvector direction; direction.sub(point,palm); const float distance=direction.magnitude();
    if(distance<.01f) return true;
    direction.div(distance); collide::rq_result hit;
    return !Level().ObjectSpace.RayPick(palm,direction,distance,collide::rqtBoth,hit,&actor)||hit.O==&object;
}
inline void updatePhysicsGrips(CActor& actor,const CopVrControls& controls,const Fmatrix& camera,bool allowed,bool secondary) {
    for(int side=0;side<2;++side) {
        auto& state=physicsGrips()[side];
        const bool native=(actor.inventory().ActiveItem()&&side==primaryHand())||
            (secondary&&side==secondaryHand())||(supportGrip().active()&&side==secondaryHand());
        Fmatrix aim; Fvector wrist;
        if(!allowed||native||!hand(controls,side,camera,aim,wrist)) { releasePhysicsGrip(side); continue; }
        const auto& input=controls.hands[side]; const Fvector palm=trackedPalm(aim,wrist,side);
        if(input.squeeze<.25f) {
            releasePhysicsGrip(side,true); physicsGrips()[side].armed=true; continue;
        }
        Fmatrix palmPose=aim; palmPose.c=palm;
        if(state.object!=u16(-1)) {
            state.motion.update(unsmoothedHandPose(palmPose,camera,side),Device.fTimeDelta);
            auto object=physicsGrabObject(state.object);
            auto shell=object?object->PPhysicsShell():nullptr;
            auto element=shell&&state.element<shell->get_ElementsNumber()?shell->get_ElementByStoreOrder(state.element):nullptr;
            if(!element||element->isFixed()) { releasePhysicsGrip(side); continue; }
            Fmatrix transform; element->get_xform(transform);
            Fvector point,error; transform.transform_tiny(point,state.local); error.sub(palm,point);
            if(!_valid(error)||error.square_magnitude()>1.f) { releasePhysicsGrip(side); continue; }
            Fvector velocity,angular,offset,rotationVelocity,targetVelocity;
            auto physical=static_cast<IPhysicsElement*>(element);
            physical->get_LinearVel(velocity); physical->get_AngularVel(angular);
            offset.sub(point,physical->mass_Center()); rotationVelocity.crossproduct(angular,offset); velocity.add(rotationVelocity);
            targetVelocity.set(0,0,0);
            if(state.targetValid&&Device.fTimeDelta>.001f) {
                targetVelocity.sub(palm,state.lastTarget).div(Device.fTimeDelta);
                const float speed=targetVelocity.magnitude(); if(speed>5.f) targetVelocity.mul(5.f/speed);
            }
            state.lastTarget=palm; state.targetValid=true;
            // A damped point spring keeps world collisions/ragdoll joints active.
            // Force is bounded; tracking jumps release rather than fling a body.
            Fvector acceleration; acceleration.mul(error,110.f);
            targetVelocity.sub(velocity); acceleration.mad(targetVelocity,18.f); acceleration.y+=9.81f;
            const float magnitude=acceleration.magnitude(); if(magnitude>80.f) acceleration.mul(80.f/magnitude);
            const float mass=_max(.1f,_min(200.f,_min(shell->getMass(),element->getMass()*3.f)));
            shell->Enable(); element->applyImpulseVsGF(point,acceleration,mass*fixed_step);
            guardReleasedCollision(actor,*object);
            occupiedHands()[side]=true; claimedHands()[side]=true; heldObjectRadius()[side]=.04f;
            continue;
        }
        if(!state.armed||input.squeeze<.7f) continue;
        state.armed=false;
        if(occupiedHands()[side]||claimedHands()[side]||beltHandBusy(side)||worldGrips()[side].pending!=u16(-1)) continue;
        xr_vector<ISpatial*> nearby;
        g_pGamePersistent->SpatialSpace.q_sphere(nearby,0,STYPE_COLLIDEABLE,palm,.3f);
        CPhysicsShellHolder* chosen=nullptr; u16 chosenElement=u16(-1); Fvector chosenPoint;
        float best=.16f;
        for(auto spatial:nearby) {
            auto object=smart_cast<CPhysicsShellHolder*>(spatial->dcast_GameObject());
            if(!object||object==&actor||physicsGrabObject(object->ID())!=object) continue;
            auto shell=object->PPhysicsShell();
            for(u16 index=0;index<shell->get_ElementsNumber();++index) {
                auto element=shell->get_ElementByStoreOrder(index); if(!element||element->isFixed()) continue;
                Fvector closest=palm;
                for(int axis=0;axis<3;++axis) {
                    Fvector direction; direction.set(0,0,0); direction[axis]=1.f;
                    float lo,hi; element->get_Extensions(direction,0,lo,hi);
                    clamp(closest[axis],lo,hi);
                }
                const float distance=palm.distance_to(closest);
                if(distance>=best||!physicsPointVisible(actor,*object,palm,closest)) continue;
                chosen=object; chosenElement=index; chosenPoint=closest; best=distance;
            }
        }
        if(!chosen) continue;
        auto element=chosen->PPhysicsShell()->get_ElementByStoreOrder(chosenElement);
        Fmatrix transform,inverse; element->get_xform(transform); inverse.invert(transform);
        state.object=chosen->ID(); state.element=chosenElement; inverse.transform_tiny(state.local,chosenPoint);
        state.lastTarget=palm; state.targetValid=true;
        state.motion.reset(); state.motion.update(unsmoothedHandPose(palmPose,camera,side),Device.fTimeDelta);
        guardReleasedCollision(actor,*chosen);
        occupiedHands()[side]=true; claimedHands()[side]=true; heldObjectRadius()[side]=.04f;
        pulse(side,.4f,45); Msg("* CoPVR: physics grip hand=%d id=%u element=%u",side,chosen->ID(),chosenElement);
    }
}
}
