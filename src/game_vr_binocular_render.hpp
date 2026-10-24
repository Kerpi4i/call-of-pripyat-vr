#pragma once
#include "game_vr_binocular.hpp"
#include "game_vr_scope_accuracy.hpp"
#include "Include/xrRender/UIRender.h"
#include "xrUICore/ui_base.h"
namespace copvr_game {
inline void renderBinocularLenses() {
    beginScopeAccuracyFrame(Device.dwFrame);
    BinocularGeometry optic; if(!binocularGeometry(optic)) return;
    auto renderer=GetModuleHandleA("xrRender_R4.dll"); using Ready=int(*)(int); using Eye=int(*)();
    auto ready=renderer?reinterpret_cast<Ready>(GetProcAddress(renderer,"copvr_optics_ready")):nullptr;
    auto currentEye=renderer?reinterpret_cast<Eye>(GetProcAddress(renderer,"copvr_current_eye")):nullptr;
    const int eye=currentEye?currentEye():0;
    static ui_shader shader[5][2];
    const int kind=optic.reticle;
    const char* shaderNames[]={"copvr_binocular","copvr_scope_pso","copvr_scope_susat","copvr_scope_g36","copvr_scope_rpg"};
    const char* shaderName=shaderNames[kind];
    if(!shader[kind][eye]->inited()) shader[kind][eye]->create(shaderName,eye?"$user$copvr_optics_right":"$user$copvr_optics_left");
    const bool imageReady=ready&&ready(eye)>0;
    // Collimated image: UV depends on the viewing ray, not a nearby flat screen.
    // 60 degree apparent field, 15 degree optical camera => approximately 4x angular zoom.
    constexpr float tangent=.577350269f;
    Fvector sharedViewer{};
    if(optic.sharedEyeValid) sharedViewer.sub(optic.sharedEye,optic.center);
    const float sharedRelief=-sharedViewer.dotproduct(optic.pose.k);
    const float sharedX=sharedViewer.dotproduct(optic.pose.i),sharedY=sharedViewer.dotproduct(optic.pose.j);
    for(int lens=0;lens<(optic.scope?1:2);++lens) {
        const float side=optic.scope?0.f:(lens==0?-1.f:1.f);
        Fvector center=optic.center; center.mad(optic.pose.i,side*optic.spacing*.5f);
        Fvector viewer; viewer.sub(Device.vCameraPosition,center);
        const float originalRelief=-viewer.dotproduct(optic.pose.k);
        Fvector lateral=viewer; lateral.mad(optic.pose.k,originalRelief);
        // Keep a near-eye aperture in front of the near clip plane. When crossed,
        // retain an opaque black stop briefly instead of exposing the model interior.
        if(originalRelief<=0&&(originalRelief<-.025f||lateral.magnitude()>.07f)) continue;
        center.mad(optic.pose.k,copvr::opticalPlaneSetback(originalRelief));
        viewer.sub(Device.vCameraPosition,center);
        const float light=imageReady&&originalRelief>.001f?copvr::binocularEyeLight({Device.vCameraPosition.x,Device.vCameraPosition.y,Device.vCameraPosition.z},
            {center.x,center.y,center.z},{optic.pose.k.x,optic.pose.k.y,optic.pose.k.z}):0.f;
        if(optic.scope&&light>0.f&&originalRelief>.001f&&Device.vCameraDirection.dotproduct(optic.pose.k)>.95f) {
            const auto pupil=copvr::scopeTubeCoordinates(0,0,viewer.dotproduct(optic.pose.i),
                viewer.dotproduct(optic.pose.j),-viewer.dotproduct(optic.pose.k),optic.radius);
            if(pupil.length()<.85f) markScopeAccuracy(optic.weapon,Device.dwTimeGlobal);
        }
        GEnv.UIRender->CacheSetXformWorld(Fidentity); GEnv.UIRender->SetShader(*shader[kind][eye]);
        GEnv.UIRender->CacheSetCullMode(IUIRender::cmNONE);
        constexpr int sectors=64,rings=8;
        GEnv.UIRender->StartPrimitive(sectors*(3+6*(rings-1)),IUIRender::ptTriList,IUIRender::pttLIT);
        auto vertex=[&](int ring,int sector) {
            float r=float(ring)/rings,angle=sector*PI_MUL_2/sectors;
            Fvector point=center; point.mad(optic.pose.i,cosf(angle)*r*optic.radius); point.mad(optic.pose.j,sinf(angle)*r*optic.radius);
            Fvector ray; ray.sub(point,Device.vCameraPosition); float axial=_max(.001f,ray.dotproduct(optic.pose.k));
            float u=.5f+ray.dotproduct(optic.pose.i)/(2*tangent*axial);
            float v=.5f-ray.dotproduct(optic.pose.j)/(2*tangent*axial);
            copvr::Vec3 tube{8,8,0};
            if(light>0.f) {
                if(optic.scope) tube=copvr::scopeTubeCoordinates(cosf(angle)*r*optic.radius,sinf(angle)*r*optic.radius,
                    viewer.dotproduct(optic.pose.i),viewer.dotproduct(optic.pose.j),-viewer.dotproduct(optic.pose.k),optic.radius);
                else if(optic.sharedEyeValid) tube=copvr::binocularSharedMask(u,v,optic.radius,sharedRelief,sharedX,sharedY);
            }
            // Shared 16-bit mask-coordinate decoder for scope tubes and binocular angular mask.
            auto pack=[](float value) { return u32((std::clamp(value,-8.f,8.f)+8.f)*(65535.f/16.f)+.5f); };
            const u32 x=pack(tube.x),y=pack(tube.y);
            const u32 colour=color_rgba(x&255,x>>8,y&255,y>>8);
            GEnv.UIRender->PushPoint(point.x,point.y,point.z,colour,u,v);
        };
        for(int sector=0;sector<sectors;++sector) {
            vertex(0,sector); vertex(1,sector); vertex(1,sector+1);
            for(int ring=1;ring<rings;++ring) {
                vertex(ring,sector); vertex(ring+1,sector); vertex(ring,sector+1);
                vertex(ring,sector+1); vertex(ring+1,sector); vertex(ring+1,sector+1);
            }
        }
        GEnv.UIRender->FlushPrimitive();
    }
    GEnv.UIRender->CacheSetCullMode(IUIRender::cmCCW);
}
}
