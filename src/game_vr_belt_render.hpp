#pragma once
#include "game_vr_body.hpp"
#include "xrGame/Level.h"
namespace copvr_game {
struct MagazineVisual { shared_str profile;unsigned instance=0;IRenderVisual* visual=nullptr; };
inline xr_vector<MagazineVisual>& magazineVisuals() { static xr_vector<MagazineVisual> value;return value; }
inline IRenderVisual* magazineVisual(const shared_str& profile,unsigned instance) {
    if(!profile.size()) return nullptr;
    for(auto& entry:magazineVisuals()) if(entry.profile==profile&&entry.instance==instance) return entry.visual;
    auto ini=equipmentConfig();if(!ini||!ini->section_exist(profile))return nullptr;
    auto visual=GEnv.Render->model_Create(ini->r_string(profile,"model"));
    magazineVisuals().push_back({profile,instance,visual});return visual;
}
inline void clearMagazineVisuals() {
    for(auto& entry:magazineVisuals()) if(entry.visual) GEnv.Render->model_Delete(entry.visual);
    magazineVisuals().clear();
}
inline void renderMagazine(const shared_str& profile,const Fmatrix& pose,u32 context,IRenderable* root,unsigned instance) {
    if(auto visual=magazineVisual(profile,instance)) {
        Fmatrix transform=pose;Fvector offset;
        transform.transform_dir(offset,equipmentConfig()->r_fvector3(profile,"center"));transform.c.sub(offset);
        if(auto k=visual->dcast_PKinematics()) k->CalculateBones(TRUE);
        GEnv.Render->add_Visual(context,root,visual,transform);
    }
}
inline void renderDetachedMagazines(u32 context,IRenderable* root) {
    for(int side=0;side<2;++side) { const auto& h=detachedMagazines()[side];if(h.held)renderMagazine(h.profile,h.pose,context,root,2+side); }
    for(int i=0;i<24;++i) { const auto& m=discardedMagazines()[i];if(m.until>Device.dwTimeGlobal)renderMagazine(m.profile,m.pose,context,root,4+i); }
}

inline void renderBeltVisual(CInventoryItem* item,const Fmatrix& anchor,float size,u32 context,IRenderable* root,bool weaponGrip=false) {
    if(!item||!item->object().Visual()) return;
    auto visual=item->object().Visual(); auto box=visual->getVisData().box;
    Fvector center,extent; box.getcenter(center); box.getsize(extent);
    if(weaponGrip) {
        center.set((box.vMin.x+box.vMax.x)*.5f,box.vMin.y+extent.y*.25f,box.vMin.z+extent.z*.28f);
        if(pSettings->line_exist(item->object().cNameSect(),"copvr_world_grip")) center=pSettings->r_fvector3(item->object().cNameSect(),"copvr_world_grip");
    }
    float scale=size>0?size/_max(.01f,_max(extent.x,_max(extent.y,extent.z))):1.f;
    Fmatrix pose=anchor; pose.i.mul(scale); pose.j.mul(scale); pose.k.mul(scale);
    Fvector offset; pose.transform_dir(offset,center); pose.c.sub(offset);
    if(auto k=visual->dcast_PKinematics()) k->CalculateBones(TRUE);
    GEnv.Render->add_Visual(context,root,visual,pose);
}
inline void renderBodyEquipment(CActor& actor,const Fmatrix& camera,u32 context,IRenderable* root) {
    Fmatrix body; if(!actor.g_Alive()||!bodyFrame(camera,body)) return;
    auto& state=bodyEquipmentState();
    renderDetachedMagazines(context,root);
    const auto& bolt=auxBolt();
    if(bolt.side>=0&&!bolt.pending) {
        auto item=smart_cast<CInventoryItem*>(Level().Objects.net_Find(bolt.item));if(item)renderBeltVisual(item,bolt.pose,0,context,root);
    }
    auto gun=smart_cast<CWeaponMagazined*>(actor.inventory().ActiveItem());
    auto hud=g_player_hud?g_player_hud->attached_item(0):nullptr;
    if(gun&&gun->VRInternalKind()==5&&hud&&hud->m_parent_hud_item==gun) {
        const auto mask=gun->VRInternalState().live;
        for(unsigned chamber=0;chamber<6;++chamber)if(mask&(1u<<chamber)) {
            Fmatrix pose;if(internalDrumRoundPose(*gun,hud,chamber,pose))renderMagazine("internal_round_vog25",pose,context,root,32+chamber);
        }
    }
    // The rear reload gesture has no belt-mounted ammo model. Only the held activator is drawn.
    for(int side=0;side<2;++side) {
        const auto& hand=state.hands[side]; if(hand.index!=BeltAmmo) continue;
        auto item=smart_cast<CInventoryItem*>(Level().Objects.net_Find(hand.item));
        auto weapon=smart_cast<CWeapon*>(Level().Objects.net_Find(hand.weapon));
        auto profile=magazineProfile(weapon);
        if(auto gun=weapon?weapon->cast_weapon_magazined():nullptr)if(gun->VRInternalKind())
            profile=equipmentConfig()->r_string(gun->VRInternalProfile(),"round_profile");
        if(hand.reloadProfile.size())profile=hand.reloadProfile;
        if(profile.size()) {
            const Fmatrix& pose=hand.pose;
            renderMagazine(profile,pose,context,root,side);
        }
        else if(item&&item->object().H_Parent()==&actor) renderBeltVisual(item,hand.pose,.10f,context,root);
    }
}
}
