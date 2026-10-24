#pragma once
#include "game_vr_arms.hpp"
#include "game_vr_launcher_config.hpp"
#include "xrGame/player_hud.h"
#include "xrCore/Animation/Motion.hpp"

namespace copvr_game {
struct ReloadGripPose { shared_str hud,hands;bool valid=false;Fmatrix objectInPalm;float gap=0; };
inline xr_vector<ReloadGripPose>& reloadGripPoses() { static xr_vector<ReloadGripPose> poses;return poses; }
struct ReloadSampleVisual {
    IRenderVisual* visual=nullptr;
    explicit ReloadSampleVisual(const char* name):visual(GEnv.Render->model_Create(name)) {}
    ~ReloadSampleVisual(){if(visual)GEnv.Render->model_Delete(visual);}
};
inline bool sampleReloadMotion(IKinematicsAnimated* animation,const MotionID& motion,float fraction) {
    if(!animation||!motion.valid())return false;
    for(u16 part=0;part<animation->partitions().count();++part) {
        auto blend=animation->PlayCycle(part,motion,FALSE);if(!blend)return false;
        blend->speed=0;blend->blendAmount=1;blend->timeCurrent=blend->timeTotal*fraction;
    }
    auto model=animation->dcast_PKinematics();model->CalculateBones_Invalidate();model->CalculateBones(TRUE);return true;
}
inline ReloadGripPose sampleReloadGrip(attachable_hud_item* hud,const shared_str& handsSection,const char* prefix="reload_grip_") {
    ReloadGripPose result;result.hud=hud->m_sect_name;result.hands=handsSection;result.objectInPalm.identity();
    const shared_str profile=(std::string(prefix)+hud->m_sect_name.c_str()).c_str();auto ini=equipmentConfig();
    if(hud->m_monolithic||!ini->section_exist(profile)||!pSettings->line_exist(handsSection,"visual")||!pSettings->line_exist(handsSection,"ancor_0"))return result;
    const auto alias=ini->r_string(profile,"alias");const auto motion=hud->m_hand_motions.find_motion(alias);
    if(!motion||motion->m_animations.empty())return result;
    ReloadSampleVisual handVisual(pSettings->r_string(handsSection,"visual")),itemVisual(hud->m_visual_name.c_str());
    auto hands=smart_cast<IKinematicsAnimated*>(handVisual.visual),item=smart_cast<IKinematicsAnimated*>(itemVisual.visual);
    if(!hands||!item)return result;
    auto hk=hands->dcast_PKinematics(),ik=item->dcast_PKinematics();
    const u16 wrist=wristBone(hk,0,BI_NONE),anchor=hk->LL_BoneID(pSettings->r_string(handsSection,"ancor_0"));
    const u16 bone=ik->LL_BoneID(ini->r_string(profile,"bone"));if(wrist==BI_NONE||anchor==BI_NONE||bone==BI_NONE)return result;
    const auto handMotion=hands->ID_Cycle_Safe(motion->m_animations[0].name);
    auto itemMotion=item->ID_Cycle_Safe(motion->m_base_name!=motion->m_additional_name?motion->m_additional_name:motion->m_animations[0].name);
    if(!itemMotion.valid())return result; // Never sample an unrelated idle pose.
    auto& root=ik->LL_GetBoneInstance(ik->LL_GetBoneRoot());root.set_callback_overwrite(TRUE);root.mTransform.identity();
    xr_vector<Fmatrix> handBind,itemBind;hk->LL_GetBindTransform(handBind);ik->LL_GetBindTransform(itemBind);
    Fmatrix inverseBone;inverseBone.invert(itemBind[bone]);
    u16 middle=BI_NONE;
    for(u16 b=0;b<hk->LL_BoneCount();++b)if(hk->GetBoneData(b).GetParentID()==wrist&&strstr(hk->LL_BoneName_dbg(b),"finger2")){middle=b;break;}
    if(middle==BI_NONE)return result;
    Fvector palmLocal,center;center.lerp(handBind[wrist].c,handBind[middle].c,.6f);
    Fmatrix inverseWrist;inverseWrist.invert(handBind[wrist]);inverseWrist.transform_tiny(palmLocal,center);
    float best=.16f;
    // Choose the closest authored grasp before insertion, not the seated pose.
    for(int frame=0;frame<12;++frame) {
        const float fraction=.15f+frame*.045f;
        if(!sampleReloadMotion(hands,handMotion,fraction)||!sampleReloadMotion(item,itemMotion,fraction))continue;
        xr_vector<Fmatrix> posed;posed.resize(hk->LL_BoneCount());
        for(u16 b=0;b<hk->LL_BoneCount();++b)posed[b]=hk->LL_GetTransform(b);
        const auto basis=palm(hk,posed,wrist,0);if(!basis.valid)continue;
        Fvector palmCenter;posed[wrist].transform_tiny(palmCenter,palmLocal);
        Fmatrix palmFrame,invPalm,attachment,delta,object;
        palmFrame.set(vec(basis.x),vec(basis.y),vec(basis.z),palmCenter);invPalm.invert(palmFrame);
        attachment.mul_43(posed[anchor],hud->m_attach_offset);
        delta.mul_43(ik->LL_GetTransform(bone),inverseBone);object.mul_43(attachment,delta);
        object.transform_tiny(center,ini->r_fvector3(profile,"center"));object.c=center;
        const float gap=center.distance_to(palmCenter);if(gap>=best)continue;
        best=gap;result.objectInPalm.mul_43(invPalm,object);result.gap=gap;result.valid=true;
    }
    Msg("* CoPVR: reload grip sampled hud=%s valid=%d palm_gap=%.3f",hud->m_sect_name.c_str(),result.valid?1:0,result.gap);
    return result;
}
inline Fmatrix reloadObjectOffset(CWeapon* weapon,int side) {
    Fmatrix offset;offset.identity();auto hud=g_player_hud?g_player_hud->attached_item(0):nullptr;
    if(!hud||hud->m_parent_hud_item!=weapon)return offset;
    const bool launcher=launcherProfile(weapon).size()!=0;
    const shared_str key=(std::string(hud->m_sect_name.c_str())+(launcher?"#launcher":"")).c_str();
    auto& poses=reloadGripPoses();const auto hands=g_player_hud->section_name();ReloadGripPose* selected=nullptr;
    for(auto& pose:poses)if(pose.hud==key&&pose.hands==hands){selected=&pose;break;}
    if(!selected){
        // Protecta has no separate shell bone. Reuse the authored 12g shell
        // contact from SPAS instead of a neutral aim-axis fallback.
        auto source=hud;
        if(hud->m_sect_name==shared_str("wpn_protecta_hud"))source=g_player_hud->create_hud_item("wpn_spas12_hud");
        auto sampled=sampleReloadGrip(source,hands,launcher?"launcher_grip_":"reload_grip_");sampled.hud=key;
        poses.push_back(sampled);selected=&poses.back();
    }
    if(!selected->valid)return offset;
    Fmatrix authored=selected->objectInPalm;
    if(side==1) {
        const auto mirrored=copvr::mirrorContactBasis(vec(authored.i),vec(authored.j),vec(authored.k));
        authored.set(vec(mirrored.x),vec(mirrored.y),vec(mirrored.z),vec(copvr::mirrorContactPoint(vec(authored.c))));
    }
    Fmatrix roll;roll.rotateZ(side==0?PI_DIV_2:-PI_DIV_2);offset.mul_43(roll,authored);return offset;
}
}
