#pragma once
#include "game_vr_zone_render.hpp"
#include "xrGame/ui/UIMainIngameWnd.h"
namespace copvr_game {
inline void wristQuad(const Fmatrix& pose,float width,float height,const ui_shader& shader,bool textured,u32 color=0xffffffff,float cropX=0.f) {
    if(textured) {
        GEnv.UIRender->CacheSetXformWorld(Fidentity); GEnv.UIRender->SetShader(*shader);
        GEnv.UIRender->CacheSetCullMode(IUIRender::cmNONE);
        GEnv.UIRender->StartPrimitive(6,IUIRender::ptTriList,IUIRender::pttLIT);
    } else beginZone(6);
    Fvector local[4]={Fvector().set(-width*.5f,0,height*.5f),Fvector().set(width*.5f,0,height*.5f),
        Fvector().set(-width*.5f,0,-height*.5f),Fvector().set(width*.5f,0,-height*.5f)};
    const float u[4]={cropX/UI_BASE_WIDTH,(256.f-cropX)/UI_BASE_WIDTH,cropX/UI_BASE_WIDTH,(256.f-cropX)/UI_BASE_WIDTH};
    const float v[4]={0,0,256.f/UI_BASE_HEIGHT,256.f/UI_BASE_HEIGHT};
    for(int index:{0,1,2,2,1,3}) {
        Fvector point; pose.transform_tiny(point,local[index]);
        GEnv.UIRender->PushPoint(point.x,point.y,point.z,color,u[index],v[index]);
    }
    GEnv.UIRender->FlushPrimitive();
}
inline void renderWristGadgets(CActor& actor,const Fmatrix& camera) {
    if(!actor.g_Alive()||actor.IsTalking()||!CurrentGameUI()||CurrentGameUI()->TopInputReceiver()||MainMenu()->IsActive()) return;
#if defined(XR_PLATFORM_WINDOWS)
    auto renderer=GetModuleHandleA("xrRender_R4.dll");
    using Begin=int(*)(int); using End=void(*)();
    auto begin=renderer?reinterpret_cast<Begin>(GetProcAddress(renderer,"copvr_wrist_begin")):nullptr;
    auto end=renderer?reinterpret_cast<End>(GetProcAddress(renderer,"copvr_wrist_end")):nullptr;
    if(!begin||!end) return;
    CopVrControls controls{}; if(!sample(controls)) return;
    static u32 updated[2]={u32(-1),u32(-1)};
    static ui_shader textures[2];
    for(int side=0;side<2;++side) {
        Fmatrix aim; Fvector grip; if(!hand(controls,side,camera,aim,grip)) continue;
        Fmatrix face=trackedGrips()[side].valid?trackedGrips()[side].pose:aim;
        face.c=grip; face.c.mad(face.k,-.12f); face.c.mad(face.j,.04f);
        {
            // Use the same wrist and anatomical calibration as the rendered hand,
            // including an equipped item's authored grip, rather than raw controller axes.
            const auto& socket=palmSockets()[side]; if(!socket.valid) continue;
            Fmatrix wrist; wrist.mul_43(aim,socket.wristFromGrip);
            Fvector palm=trackedPalm(aim,grip,side),offset;
            wrist.transform_dir(offset,socket.boneLocal); wrist.c.sub(palm,offset);
            if(nativeWrists()[side].valid) wrist=nativeWrists()[side].pose;
            if(side==secondaryHand()) { Fmatrix locked; if(lockedSupport(locked)) wrist=locked; }
            Fmatrix inverseSocket,roll,inverseCalibration,anatomical;
            inverseSocket.invert(socket.wristFromGrip); roll.rotateZ(side==0?PI_DIV_2:-PI_DIV_2);
            inverseCalibration.mul_43(inverseSocket,roll); anatomical.mul_43(wrist,inverseCalibration);
            face=anatomical; // +anatomical Y is the dorsal side of this skeleton.
            wrist.transform_tiny(face.c,socket.boneLocal);
            face.c.mad(face.j,.025f); // just above the back of the palm, not the forearm
            Fmatrix turn,rotated; turn.rotateY(side==0?-PI_DIV_2:PI_DIV_2); rotated.mul_43(face,turn); face=rotated;
        }
        Fvector viewer; viewer.sub(Device.vCameraPosition,face.c); viewer.normalize_safe();
        if(face.j.dotproduct(viewer)<.08f) continue; // dorsal face, not a floating HUD
        if(updated[side]!=Device.dwFrame) {
            if(!begin(side)) continue;
            const auto pointType=UI().m_currentPointType; UI().m_currentPointType=IUIRender::pttTL;
            CurrentGameUI()->UIMainIngameWnd->DrawWristWidget(side);
            UI().m_currentPointType=pointType; end(); updated[side]=Device.dwFrame;
        }
        if(!textures[side]->inited()) textures[side]->create(side?"copvr_minimap":"copvr_wrist",side?"$user$copvr_wrist1":"$user$copvr_wrist0");
        const float width=.09f,height=.09f;
        face.c.mad(face.j,.001f);
        // Native circular radar is narrowed for widescreen UI. Crop its empty
        // horizontal margins before mapping to a square in world metres.
        const float cropX=128.f*(1.f-UI().get_current_kx());
        wristQuad(face,width,height,textures[side],true,0xffffffff,cropX);
    }
    GEnv.UIRender->CacheSetCullMode(IUIRender::cmCCW);
#endif
}
}
