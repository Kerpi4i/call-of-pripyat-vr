#pragma once
#include "arm_ik.hpp"
#include "hand_basis.hpp"
#include "game_vr_controls.hpp"
#include "xrCore/Animation/Bone.hpp"
namespace copvr_game {
inline copvr::Vec3 vec(const Fvector& v) { return {v.x,v.y,v.z}; }
inline Fvector vec(copvr::Vec3 v) { Fvector r; r.set(v.x,v.y,v.z); return r; }
inline Fmatrix rotateSegment(const Fmatrix& source,copvr::Vec3 oldDirection,copvr::Vec3 newDirection,copvr::Vec3 position) {
    auto q=copvr::rotationBetween(oldDirection,newDirection);
    Fmatrix result=source;
    result.i=vec(copvr::rotate(q,vec(source.i)));
    result.j=vec(copvr::rotate(q,vec(source.j)));
    result.k=vec(copvr::rotate(q,vec(source.k)));
    result.c=vec(position); return result;
}
struct TrackedHandPalette {const IKinematics* owner=nullptr;u32 frame=0;xr_vector<Fmatrix> matrices;};
inline TrackedHandPalette& trackedHandPalette(){static TrackedHandPalette value;return value;}
struct BoneOverride {
    CBoneInstance* bone{}; Fmatrix target;
    BoneCallback previous{}; void* parameter{}; u32 type{}; BOOL overwrite{};
    static void apply(CBoneInstance* bone) { bone->mTransform=*static_cast<Fmatrix*>(bone->callback_param()); }
    void set(CBoneInstance& b,const Fmatrix& matrix) {
        bone=&b; target=matrix; previous=b.callback(); parameter=b.callback_param();
        type=b.callback_type(); overwrite=b.callback_overwrite();
        b.set_callback(type,apply,&target,TRUE);
    }
    ~BoneOverride() { if(bone) bone->set_callback(type,previous,parameter,overwrite); }
};
inline u16 wristBone(IKinematics* model,int side,u16 anchor) {
    auto id=model->LL_BoneID(side==0?"l_hand":"r_hand");
    if(id==BI_NONE) id=model->LL_BoneID(side==0?"bip01_l_hand":"bip01_r_hand");
    return id==BI_NONE?anchor:id;
}
inline copvr::PalmBasis palm(IKinematics* model,const xr_vector<Fmatrix>& bind,u16 wrist,int side) {
    u16 knuckles[3]={BI_NONE,BI_NONE,BI_NONE};
    const char* names[]={"finger1","finger2","finger4"};
    for(u16 bone=0;bone<model->LL_BoneCount();++bone) {
        if(model->GetBoneData(bone).GetParentID()!=wrist) continue;
        for(int i=0;i<3;++i) if(strstr(model->LL_BoneName_dbg(bone),names[i])) knuckles[i]=bone;
    }
    if(knuckles[0]==BI_NONE||knuckles[1]==BI_NONE||knuckles[2]==BI_NONE) return {};
    return copvr::palmBasis(vec(bind[wrist].c),vec(bind[knuckles[0]].c),vec(bind[knuckles[1]].c),vec(bind[knuckles[2]].c),side);
}
inline bool oppositeWrist(IKinematics* model,const Fmatrix& source,int from,int to,Fmatrix& target) {
    xr_vector<Fmatrix> bind; model->LL_GetBindTransform(bind);
    const auto a=wristBone(model,from,BI_NONE),b=wristBone(model,to,BI_NONE);
    if(a==BI_NONE||b==BI_NONE) return false;
    const auto pa=palm(model,bind,a,from),pb=palm(model,bind,b,to);
    if(!pa.valid||!pb.valid) return false;
    Fmatrix palmA,palmB,bindA=bind[a],bindB=bind[b],inverseA,inversePalmB,toAnatomical,fromAnatomical,canonical;
    Fvector zero; zero.set(0,0,0); bindA.c=bindB.c=zero;
    palmA.set(vec(pa.x),vec(pa.y),vec(pa.z),zero);
    palmB.set(vec(pb.x),vec(pb.y),vec(pb.z),zero);
    inverseA.invert(bindA); inversePalmB.invert(palmB);
    toAnatomical.mul_43(inverseA,palmA); fromAnatomical.mul_43(inversePalmB,bindB);
    Fmatrix mirrored;
    canonical.mul_43(source,toAnatomical);
    const auto reflected=copvr::mirrorContactBasis(vec(canonical.i),vec(canonical.j),vec(canonical.k));
    mirrored.set(vec(reflected.x),vec(reflected.y),vec(reflected.z),vec(copvr::mirrorContactPoint(vec(canonical.c))));
    target.mul_43(mirrored,fromAnatomical);
    if(palmSockets()[from].valid&&palmSockets()[to].valid) {
        Fvector sourcePalm,targetPalm,expected,shift;
        source.transform_tiny(sourcePalm,palmSockets()[from].boneLocal);
        target.transform_tiny(targetPalm,palmSockets()[to].boneLocal);
        expected=vec(copvr::mirrorContactPoint(vec(sourcePalm)));
        shift.sub(expected,targetPalm); target.c.add(shift);
    }
    return true;
}
inline bool retargetFinger(IKinematics* model,const xr_vector<Fmatrix>& bind,u16 bone,int from,int to,Fmatrix& local) {
    if(from==to) {
        const auto parent=model->GetBoneData(bone).GetParentID(); if(parent==BI_NONE) return false;
        Fmatrix inverse; inverse.invert(model->LL_GetTransform(parent));
        local.mul_43(inverse,model->LL_GetTransform(bone)); return true;
    }
    string256 name; xr_strcpy(name,model->LL_BoneName_dbg(bone));
    char* marker=strstr(name,to==0?"_l_":"_r_");
    if(marker) ++marker;
    else if(name[0]==(to==0?'l':'r')&&name[1]=='_') marker=name;
    if(!marker) return false;
    *marker=from==0?'l':'r';
    const auto source=model->LL_BoneID(name); if(source==BI_NONE) return false;
    const auto sourceParent=model->GetBoneData(source).GetParentID(),targetParent=model->GetBoneData(bone).GetParentID();
    const auto sourceWrist=wristBone(model,from,BI_NONE),targetWrist=wristBone(model,to,BI_NONE);
    if(sourceParent==BI_NONE||targetParent==BI_NONE||sourceWrist==BI_NONE||targetWrist==BI_NONE) return false;
    const auto a=palm(model,bind,sourceWrist,from),b=palm(model,bind,targetWrist,to);
    if(!a.valid||!b.valid) return false;
    Fmatrix sourceFrame,targetFrame,inverseTarget,reflection,temp,mapping;
    sourceFrame.set(vec(a.x),vec(a.y),vec(a.z),bind[sourceWrist].c);
    targetFrame.set(vec(b.x),vec(b.y),vec(b.z),bind[targetWrist].c);
    inverseTarget.invert(targetFrame); reflection.identity(); reflection._11=-1.f;
    temp.mul_43(sourceFrame,reflection); mapping.mul_43(temp,inverseTarget);
    Fmatrix inverseSource,parentMap,boneMap,inverseParentMap,animatedInverse,animatedLocal,posed;
    inverseSource.invert(bind[sourceParent]); temp.mul_43(inverseSource,mapping); parentMap.mul_43(temp,bind[targetParent]);
    inverseSource.invert(bind[source]); temp.mul_43(inverseSource,mapping); boneMap.mul_43(temp,bind[bone]);
    animatedInverse.invert(model->LL_GetTransform(sourceParent));
    animatedLocal.mul_43(animatedInverse,model->LL_GetTransform(source));
    inverseParentMap.invert(parentMap); posed.mul_43(inverseParentMap,animatedLocal); local.mul_43(posed,boneMap);
    // Retarget rotations, retaining the target mesh's bone lengths.
    Fmatrix inverseBind,rest; inverseBind.invert(bind[targetParent]); rest.mul_43(inverseBind,bind[bone]); local.c=rest.c;
    return true;
}
inline void configurePalms(IKinematics* model) {
    xr_vector<Fmatrix> bind; model->LL_GetBindTransform(bind);
    for(int side=0;side<2;++side) {
        auto& socket=palmSockets()[side]; socket.valid=false;
        const auto wrist=wristBone(model,side,BI_NONE); if(wrist==BI_NONE) continue;
        const auto basis=palm(model,bind,wrist,side); if(!basis.valid) continue;
        u16 middle=BI_NONE;
        for(u16 bone=0;bone<model->LL_BoneCount();++bone)
            if(model->GetBoneData(bone).GetParentID()==wrist&&strstr(model->LL_BoneName_dbg(bone),"finger2")) { middle=bone; break; }
        if(middle==BI_NONE) continue;
        Fvector center; center.lerp(bind[wrist].c,bind[middle].c,.6f);
        Fmatrix inverseWrist,anatomical,inverseAnatomical,calibration,roll,pose;
        inverseWrist.invert(bind[wrist]); inverseWrist.transform_tiny(socket.boneLocal,center);
        anatomical.set(vec(basis.x),vec(basis.y),vec(basis.z),bind[wrist].c);
        inverseAnatomical.invert(anatomical); calibration.mul_43(inverseAnatomical,bind[wrist]);
        roll.rotateZ(side==0?PI_DIV_2:-PI_DIV_2); pose.mul_43(roll,calibration);
        socket.wristFromGrip=pose;
        Fvector zero; zero.set(0,0,0);
        socket.aimOffset=equipmentVector("hands",side==0?"left_palm":"right_palm",zero);
        socket.normal.invert(roll.j); socket.valid=true;
    }
}
struct FingerOverride {
    BoneOverride saved; IKinematics* model{}; u16 parent{}; Fmatrix local;
    static void apply(CBoneInstance* bone) {
        auto self=static_cast<FingerOverride*>(bone->callback_param());
        bone->mTransform.mul_43(self->model->LL_GetTransform(self->parent),self->local);
    }
    void set(IKinematics* k,u16 bone,const Fmatrix& value) {
        model=k; parent=k->GetBoneData(bone).GetParentID(); local=value;
        auto& b=k->LL_GetBoneInstance(bone); saved.set(b,b.mTransform);
        b.set_callback(b.callback_type(),apply,this,TRUE);
    }
};
// Overrides exist only during synchronous CalculateBones; no stack pointer is
// retained by the renderer or by the next simulation frame.
inline bool arms(IKinematics* model,const Fmatrix& modelWorld,const Fmatrix& camera,
    const CopVrControls& controls,u16 rightAnchor,u16 leftAnchor) {
    if(!model) return false;
    Fmatrix inverseModel; inverseModel.invert(modelWorld);
    BoneOverride overrides[2]; bool changed=false;
    std::array<FingerOverride,128> fingers; size_t fingerCount=0;
    xr_vector<Fmatrix> bind; model->LL_GetBindTransform(bind);
    u16 wrists[2]={BI_NONE,BI_NONE}; Fvector targets[2];
    for(int side=0;side<2;++side) {
        // The HUD section defines attachment bones for its own skeleton.
        // Stock CoP uses different names from the world actor skeleton.
        auto wrist=wristBone(model,side,side==0?leftAnchor:rightAnchor);
        if(wrist==BI_NONE) continue;
        Fmatrix aim; Fvector grip;
        // Restore the previously validated neutral pointing frame for free
        // hands. Raw grip axes are not the bind-pose finger axes.
        if(!hand(controls,side,camera,aim,grip)) { model->LL_SetBoneVisible(wrist,FALSE,TRUE); continue; }
        const Fmatrix oldWrist=bind[wrist];
        auto basis=palm(model,bind,wrist,side);
        if(!basis.valid) { if(Device.dwFrame%300==0) Msg("! CoPVR: missing palm basis hand=%d",side); continue; }
        Fmatrix anatomical,inverseAnatomical,calibration,desiredWorld,desiredLocal;
        anatomical.set(vec(basis.x),vec(basis.y),vec(basis.z),bind[wrist].c);
        inverseAnatomical.invert(anatomical); calibration.mul_43(inverseAnatomical,oldWrist);
        // Neutral Touch grip: thumbs up and palms facing inward. Opposite
        // rolls are a mirrored anatomical convention, independent of bone axes.
        Fmatrix roll,palmAim; roll.rotateZ(side==0?PI_DIV_2:-PI_DIV_2);
        palmAim.mul_43(aim,roll);
        desiredWorld.mul_43(palmAim,calibration);
        Fvector palmPosition=trackedPalm(aim,grip,side),palmOffset;
        desiredWorld.transform_dir(palmOffset,palmSockets()[side].boneLocal);
        desiredWorld.c.sub(palmPosition,palmOffset); grip=desiredWorld.c;
        if(nativeWrists()[side].valid) {
            desiredWorld=nativeWrists()[side].pose; grip=desiredWorld.c;
        }
        // A supported offhand is a rigid weapon attachment, including roll;
        // the raw Touch rotation only applies again after releasing support.
        if(side==secondaryHand()) {
            Fmatrix locked;
            if(lockedSupport(locked)) { desiredWorld=locked; grip=locked.c; }
        }
        if(partGripAnchor().active&&partGripAnchor().side==side) {
            Fvector offset;desiredWorld.transform_dir(offset,palmSockets()[side].boneLocal);
            desiredWorld.c.sub(partGripAnchor().palm,offset);grip=desiredWorld.c;
        }
        desiredLocal.mul_43(inverseModel,desiredWorld);
        for(u16 bone=0;bone<model->LL_BoneCount()&&fingerCount<fingers.size();++bone) {
            auto parent=model->GetBoneData(bone).GetParentID();
            auto ancestor=parent; bool descendant=false;
            for(u16 depth=0;ancestor!=BI_NONE&&depth<model->LL_BoneCount();++depth) {
                if(ancestor==wrist) { descendant=true; break; }
                ancestor=model->GetBoneData(ancestor).GetParentID();
            }
            if(!descendant||parent==BI_NONE) continue;
            const char* name=model->LL_BoneName_dbg(bone);
            if(!strstr(name,"finger")) continue;
            Fmatrix inv,rest;
            inv.invert(bind[parent]); rest.mul_43(inv,bind[bone]);
            const bool indexFinger=strstr(name,"finger1")||strstr(name,"index");
            const bool supported=side==secondaryHand()&&supportGrip().active();
            const bool worldHeld=occupiedHands()[side];
            const bool nativeHeld=nativeHeldHands()[side];
            if(nativeHeld||supported) {
                // Primary poses come from the authored right weapon hand;
                // foregrip poses from the authored left. Retarget bone axes
                // for the opposite hand instead of replacing the grip by curl.
                Fmatrix authored;
                if(retargetFinger(model,bind,bone,supported?0:nativeWrists()[side].source,side,authored)) {
                    fingers[fingerCount++].set(model,bone,authored);
                    continue;
                }
            }
            float curl=indexFinger?controls.hands[side].trigger:controls.hands[side].squeeze;
            // A grip wraps the index too; releasing the trigger while holding a
            // box or foregrip must not straighten that finger through the mesh.
            if(worldHeld||supported) curl=_max(curl,controls.hands[side].squeeze*.85f);
            if(nativeHeld) curl=_max(curl,indexFinger?.12f:.65f);
            if(worldHeld) {
                const float limit=_max(.4f,_min(.95f,1.05f-heldObjectRadius()[side]*4.f));
                curl=_min(curl,limit);
            }
            clamp(curl,0.f,1.f);
            const bool thumb=strstr(name,"finger0")!=nullptr;
            Fvector axis; inv.transform_dir(axis,vec(thumb?basis.y*(side==0?-1.f:1.f):basis.x)); axis.normalize_safe();
            const bool proximal=parent==wrist;
            const bool middle=!proximal&&model->GetBoneData(parent).GetParentID()==wrist;
            const float jointLimit=thumb?.50f:proximal?1.0f:middle?1.1f:.75f;
            Fmatrix rotation,local; rotation.rotation(axis,curl*jointLimit);
            local.mul_43(rotation,rest); local.c=rest.c;
            fingers[fingerCount++].set(model,bone,local);
        }
        overrides[side].set(model->LL_GetBoneInstance(wrist),desiredLocal);
        wrists[side]=wrist; targets[side]=grip;
        changed=true;
    }
    if(changed) {
        model->CalculateBones_Invalidate(); model->CalculateBones(TRUE);
        // Collapse only the sleeve skinning palette to the nearest wrist.
        // Keep bone transforms intact for item attachments and next animation
        // evaluation; never zero a parent transform of the tracked fingers.
        for(u16 bone=0;bone<model->LL_BoneCount();++bone) {
            bool glove=false; auto ancestor=bone;
            for(u16 depth=0;ancestor!=BI_NONE&&depth<model->LL_BoneCount();++depth) {
                if(ancestor==wrists[0]||ancestor==wrists[1]) { glove=true; break; }
                ancestor=model->GetBoneData(ancestor).GetParentID();
            }
            if(glove) continue;
            int side=0;
            if(wrists[0]==BI_NONE) side=1;
            else if(wrists[1]!=BI_NONE&&bind[bone].c.distance_to(bind[wrists[1]].c)<bind[bone].c.distance_to(bind[wrists[0]].c)) side=1;
            if(wrists[side]==BI_NONE) continue;
            auto& render=model->LL_GetBoneInstance(bone).mRenderTransform;
            render.scale(0.f,0.f,0.f); render.c=model->LL_GetTransform(wrists[side]).c;
        }
    }
    if(changed) {
        auto& palette=trackedHandPalette();palette.owner=model;palette.frame=Device.dwFrame;palette.matrices.resize(model->LL_BoneCount());
        for(u16 bone=0;bone<model->LL_BoneCount();++bone)palette.matrices[bone]=model->LL_GetBoneInstance(bone).mRenderTransform;
    }
    if(changed && Device.dwFrame%300==0) for(int side=0;side<2;++side) {
        if(wrists[side]==BI_NONE) continue;
        Fvector actual; modelWorld.transform_tiny(actual,model->LL_GetTransform(wrists[side]).c);
        Msg("* CoPVR: IK hand=%d wrist=%s error_m=%.5f",side,model->LL_BoneName_dbg(wrists[side]),actual.distance_to(targets[side]));
    }
    return changed;
}
}
