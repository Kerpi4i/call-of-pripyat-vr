#pragma once
#include "game_vr_body.hpp"
#include "game_vr_weapon_parts.hpp"
#include "xrGame/Level.h"
#include "Include/xrRender/UIRender.h"
#include "Include/xrRender/UIShader.h"
#include "xrUICore/ui_defs.h"
extern Flags32 copvrInteractionZoneFlags;
namespace copvr_game {
inline bool debugZonesEnabled() { return copvrInteractionZoneFlags.test(1u); }
inline void zoneLine(const Fvector& from,const Fvector& to,u32 color) {
    Fvector direction,view,width;
    direction.sub(to,from); view.sub(Device.vCameraPosition,from);
    width.crossproduct(direction,view); width.normalize_safe(); width.mul(.0025f);
    Fvector points[4]; points[0].sub(from,width); points[1].add(from,width);
    points[2].sub(to,width); points[3].add(to,width);
    for(int j:{0,1,2,2,1,3}) GEnv.UIRender->PushPoint(points[j].x,points[j].y,points[j].z,color,0,0);
}
inline void beginZone(u32 vertices) {
    static ui_shader shader;
    if(!shader->inited()) shader->create("copvr_zones","$null");
    GEnv.UIRender->CacheSetXformWorld(Fidentity);
    GEnv.UIRender->SetShader(*shader);
    GEnv.UIRender->CacheSetCullMode(IUIRender::cmNONE);
    GEnv.UIRender->StartPrimitive(vertices,IUIRender::ptTriList,IUIRender::pttLIT);
}
inline void renderZoneSphere(const Fmatrix& frame,const Fvector& local,float radius,u32 color) {
    beginZone(3*32*6);
    for(int axis=0;axis<3;++axis) for(int segment=0;segment<32;++segment) {
        Fvector points[2];
        for(int end=0;end<2;++end) {
            const float angle=PI_MUL_2*(segment+end)/32;
            points[end]=local;
            points[end][(axis+1)%3]+=radius*cosf(angle); points[end][(axis+2)%3]+=radius*sinf(angle);
            frame.transform_tiny(points[end]);
        }
        zoneLine(points[0],points[1],color);
    }
    GEnv.UIRender->FlushPrimitive();
}
inline void renderZoneBox(const Fmatrix& frame,const Fvector& center,const Fvector& half,u32 color) {
    Fvector corners[8];
    for(int i=0;i<8;++i) {
        corners[i].set(center.x+((i&1)?half.x:-half.x),center.y+((i&2)?half.y:-half.y),center.z+((i&4)?half.z:-half.z));
        frame.transform_tiny(corners[i]);
    }
    beginZone(12*6);
    for(int i=0;i<8;++i) for(int bit:{1,2,4}) if(!(i&bit)) zoneLine(corners[i],corners[i|bit],color);
    GEnv.UIRender->FlushPrimitive();
}
inline void renderInteractionZones(CActor& actor,const Fmatrix& camera) {
    if(!debugZonesEnabled()||!actor.g_Alive()||actor.IsTalking()||!CurrentGameUI()||CurrentGameUI()->TopInputReceiver()||MainMenu()->IsActive()) return;
    auto ini=equipmentConfig();
    const bool other=!(ini&&ini->line_exist("body","debug_other_zones"))||ini->r_bool("body","debug_other_zones");
    Fmatrix body; if(!bodyFrame(camera,body)) return;
    Fmatrix chest; if(!chestFrame(camera,chest)) return;
    renderZoneBox(chest,chestZoneCenter(),chestZoneHalf(),color_rgba(50,215,255,180));
    if(other) renderZoneBox(body,backToolsCenter(),backToolsHalf(),color_rgba(255,165,50,180));
    for(float side:{-1.f,1.f}) renderZoneBox(body,Fvector().set(side*copvr::chestHalfX*.5f,copvr::quickHeight,copvr::chestForward),
        Fvector().set(copvr::chestHalfX*.5f,copvr::chestHalfY,copvr::chestHalfZ),color_rgba(255,145,185,180));
    Fmatrix head;
    if(headGestureFrame(camera,head)) {
        renderZoneBox(head,Fvector().set(0,.20f+copvr::weaponZoneHeightOffset,-.24f+copvr::weaponZoneForwardOffset),Fvector().set(.46f,.20f,.26f),color_rgba(185,90,255,170));
        // Front extension is used only when returning a weapon.
        renderZoneBox(head,Fvector().set(0,.20f+copvr::weaponZoneHeightOffset,.14f+copvr::weaponZoneForwardOffset),Fvector().set(.46f,.20f,.12f),color_rgba(225,155,255,110));
        if(other) for(float side:{-1.f,1.f}) renderZoneBox(head,Fvector().set(side*copvr::templeSide,0,copvr::templeForward),Fvector().set(copvr::templeHalfX,copvr::templeHalfY,copvr::templeHalfZ),color_rgba(255,245,90,180));
    }
    // Show weapon-bound zones even before a reload proxy is held.
    if(other) {
        auto weapon=smart_cast<CWeapon*>(actor.inventory().ActiveItem());
        auto hud=g_player_hud?g_player_hud->attached_item(0):nullptr;
        const auto& profile=gripProfile();
        if(weapon&&hud&&hud->m_parent_hud_item==weapon&&profile.valid&&profile.weapon==weapon) {
            auto gun=weapon->cast_weapon_magazined();
            if(launcherProfile(weapon).size()) {
                Fvector local,world;if(launcherPort(weapon,local,world))renderZoneSphere(hud->m_item_transform,local,.075f,color_rgba(70,255,120,190));
            } else if(gun&&gun->VRInternalKind()) {
                for(int i=0;i<(gun->VRInternalKind()==2?2:1);++i) {
                    Fvector local,world;if(internalPort(*gun,i,local,world))
                        renderZoneSphere(hud->m_item_transform,local,gun->VRInternalKind()==3?.10f:.065f,color_rgba(70,255,120,190));
                }
            } else {
                Fvector port=profile.right,offset;offset.set(0,-.07f,.05f);
                if(pSettings->line_exist(weapon->cNameSect(),"copvr_reload_offset"))offset=pSettings->r_fvector3(weapon->cNameSect(),"copvr_reload_offset");
                port.add(offset);Fvector worldPort;magazinePort(weapon,port,worldPort);
                renderZoneSphere(hud->m_item_transform,port,copvr::reloadRadius,color_rgba(70,255,120,190));
            }
            if(gun&&gun->VRInternalKind()==5) {
                Fvector latch;internalBodyDelta(hud).transform_tiny(latch,ini->r_fvector3(gun->VRInternalProfile(),"latch"));
                renderZoneSphere(hud->m_item_transform,latch,.055f,color_rgba(255,200,70,200));
            }
            if(gun&&gun->VRInternalKind()==2) {
                Fvector contact;if(internalBreakContact(*gun,hud,contact))
                    renderZoneSphere(hud->m_item_transform,contact,.045f,color_rgba(255,200,70,200));
            }
            if(profile.supportEnabled) {
                const auto& part=articulatedSupport();
                renderZoneSphere(hud->m_item_transform,part.weapon==weapon&&part.pointValid?part.point:profile.foregrip,.11f,color_rgba(80,150,255,160));
            }
            if(hud->m_model) {
                u16 bone;Fvector contact,axis;float travel;
                if((!gun||gun->VRInternalKind()<2)&&handleContact(weapon,hud,bone,contact,axis,travel))
                    renderZoneSphere(hud->m_item_transform,contact,.065f,color_rgba(255,240,120,150));
            }
        }
    }
    GEnv.UIRender->CacheSetCullMode(IUIRender::cmCCW);
}
}
