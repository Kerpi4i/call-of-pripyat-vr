#pragma once
#include "game_vr_controls.hpp"
#include "game_vr_optics_pass.hpp"
#include "binocular_optics.hpp"
#include "game_vr_navigation.hpp"
#include "vr_optics_api.hpp"
#include "xrGame/Actor.h"
#include "xrGame/Level.h"
#include "Include/xrRender/Kinematics.h"
#include "xrGame/ActorEffector.h"
#include "xrGame/Inventory.h"
#include "xrGame/WeaponBinoculars.h"
#include "xrGame/player_hud.h"
#include "xrGame/UIGameCustom.h"
#include "xrGame/MainMenu.h"
namespace copvr_game {
struct BinocularGeometry { Fmatrix pose; Fvector center,cameraCenter,sharedEye{}; float spacing=.065f,radius=.018f; bool scope=false,sharedEyeValid=false; int reticle=0; const void* weapon{}; };
inline bool binocularGeometry(BinocularGeometry& result) {
    if(!strstr(Core.Params,"-copvr")||!g_pGameLevel||!g_player_hud||opticsPass()) return false;
    auto actor=smart_cast<CActor*>(Level().CurrentViewEntity());
    if(!actor||!actor->g_Alive()||actor->IsTalking()||MainMenu()->IsActive()||!CurrentGameUI()||CurrentGameUI()->TopInputReceiver()) return false;
    CopVrControls controls{};
    if(!sample(controls)||!controls.active||!controls.hands[primaryHand()].valid) return false;
    const auto& solved=hudPoseReady(); if(solved.owner!=g_player_hud||!solved.ready) return false;
    auto item=smart_cast<CWeapon*>(actor->inventory().ActiveItem());
    auto hud=g_player_hud->attached_item(0);
    if(!item||!hud||hud->m_parent_hud_item!=item->cast_hud_item()||!hud->m_model) return false;
    result.weapon=item;
    result.scope=smart_cast<CWeaponBinoculars*>(item)==nullptr;
    string256 section; xr_strcpy(section,"binocular_vr");
    u16 scopeBone=BI_NONE;
    if(result.scope) {
        const bool permanent=item->get_ScopeStatus()==ALife::eAddonPermanent;
        if((!item->ScopeAttachable()&&!permanent)||!item->IsScopeAttached()) return false;
        const shared_str source=permanent?item->cNameSect():item->GetScopeName();
        const char* texture=pSettings->line_exist(source,"scope_texture")?pSettings->r_string(source,"scope_texture"):"wpn_crosshair";
        result.reticle=!xr_strcmp(texture,"wpn_crosshair_l85")?2:
            (!xr_strcmp(texture,"wpn_crosshair_g36")?3:(!xr_strcmp(texture,"wpn_crosshair_rpg")?4:1));
        xr_strcpy(section,"scope_vr_"); xr_strcat(section,item->HudSection().c_str());
        auto ini=equipmentConfig(); if(!ini||!ini->section_exist(section)) return false;
        const char* boneName=ini->line_exist(section,"scope_bone")?ini->r_string(section,"scope_bone"):"wpn_scope";
        scopeBone=hud->m_model->LL_BoneID(boneName);
        if(scopeBone==BI_NONE||!hud->m_model->LL_GetBoneVisible(scopeBone)) return false;
    }
    const auto box=hud->m_model->dcast_RenderVisual()->getVisData().box;
    Fvector center; box.getcenter(center); center.z=box.vMax.z+.003f;
    Fmatrix modelPose=hud->m_item_transform;
    if(result.scope) {
        const u16 bone=scopeBone;
        xr_vector<Fmatrix> bind; hud->m_model->LL_GetBindTransform(bind);
        if(bone>=bind.size()) return false;
        Fmatrix inverseBind,delta; inverseBind.invert(bind[bone]);
        delta.mul_43(hud->m_model->LL_GetTransform(bone),inverseBind);
        modelPose.mul_43(hud->m_item_transform,delta);
    }
    result.pose=equipmentPose(section,"optics",modelPose);
    center=equipmentVector(section,"lens_center",center);
    result.pose.transform_tiny(result.center,center);
    result.cameraCenter=result.center;
    // Oculars face local +Z; the optical camera looks through the model toward -Z.
    // Turn the optical basis after locating the lenses to preserve model-space offsets.
    if(!result.scope) { result.pose.i.invert(); result.pose.k.invert(); }
    auto ini=equipmentConfig();
    if(ini&&ini->line_exist(section,"lens_spacing")) result.spacing=ini->r_float(section,"lens_spacing");
    if(ini&&ini->line_exist(section,"lens_radius")) result.radius=ini->r_float(section,"lens_radius");
    result.spacing=_max(.02f,_min(.10f,result.spacing)); result.radius=_max(.005f,_min(.035f,result.radius));
    // Use the head centre for both apertures so the two eyes share one physical size.
    CopVrHead head{}; head.abi=COPVR_HEAD_ABI;
    auto bridge=GetModuleHandleA("copvr_bridge.dll");
    auto get=bridge?reinterpret_cast<CopVrGetHead>(GetProcAddress(bridge,"copvr_get_head")):nullptr;
    if(!result.scope&&get&&get(&head)>0&&head.active) {
        Fmatrix base; actor->Cameras().camera_Matrix(base);
        Fvector eyes; base.transform_tiny(eyes,Fvector().set(head.position[0],head.position[1],head.position[2]));
        result.sharedEye=eyes; result.sharedEyeValid=true;
        const float distance=eyes.distance_to(result.center);
        result.radius=copvr::binocularApertureRadius(result.radius,distance);
        result.center.mad(result.pose.k,-.02f*copvr::binocularProximity(distance));
    }
    return _valid(result.center)&&_valid(result.pose.k);
}
inline int binocularView(CopVrOpticsView* out) {
    if(!out||out->abi!=1) return 0;
    BinocularGeometry optic; if(!binocularGeometry(optic)) return 0;
    CopVrHead head{}; head.abi=COPVR_HEAD_ABI;
    auto bridge=GetModuleHandleA("copvr_bridge.dll");
    auto get=bridge?reinterpret_cast<CopVrGetHead>(GetProcAddress(bridge,"copvr_get_head")):nullptr;
    if(!get||get(&head)<=0||!head.active) return 0;
    auto actor=smart_cast<CActor*>(Level().CurrentViewEntity()); Fmatrix base; actor->Cameras().camera_Matrix(base);
    Fvector eye,look; base.transform_tiny(eye,Fvector().set(head.position[0],head.position[1],head.position[2]));
    base.transform_dir(look,Fvector().set(head.direction[0],head.direction[1],head.direction[2]));
    if(!copvr::binocularNear({eye.x,eye.y,eye.z},{optic.cameraCenter.x,optic.cameraCenter.y,optic.cameraCenter.z},{optic.pose.k.x,optic.pose.k.y,optic.pose.k.z},{look.x,look.y,look.z})) return 0;
    Fvector origin=optic.cameraCenter; origin.mad(optic.pose.k,.18f);
    out->position[0]=origin.x; out->position[1]=origin.y; out->position[2]=origin.z;
    for(int i=0;i<3;++i) { out->direction[i]=optic.pose.k[i]; out->up[i]=optic.pose.j[i]; }
    out->fov=15.f;
    auto ini=equipmentConfig(); if(ini&&ini->line_exist("binocular_vr","vertical_fov")) out->fov=ini->r_float("binocular_vr","vertical_fov");
    out->fov=_max(3.f,_min(45.f,out->fov));
    if(optic.scope) return COPVR_OPTICS_SCOPE; // Fixed prototype optics; no proximity animation or stick zoom.
    static copvr::BinocularZoom zoom;
    CopVrNavigation nav{};
    const bool navActive=navigation(nav)&&nav.active;
    const float factor=zoom.update(navActive,nav.scrollY,nav.timestampMs);
    out->fov=rad2deg(2.f*atanf(tanf(deg2rad(out->fov)*.5f)/factor));
    return COPVR_OPTICS_BINOCULAR;
}
}
