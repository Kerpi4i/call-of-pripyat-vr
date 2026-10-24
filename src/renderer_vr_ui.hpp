#pragma once
// Included inside the D3D11 renderer namespace, with renderer types available.
#if defined(USE_DX11)
static ID3D11Texture2D* copvrUITexture{};
static ID3D11RenderTargetView* copvrUIRT{};
static ID3D11RenderTargetView* copvrSavedRT[4]{};
static ID3D11DepthStencilView* copvrSavedZB{};
static ref_rt copvrWristRT[2],copvrOpticsRT[2];
static bool copvrOpticsPassActive=false; static int copvrOpticsEye=0;
static u32 copvrOpticsFrame[2]={u32(-1),u32(-1)};
extern "C" __declspec(dllexport) int copvr_optics_pass() { return copvrOpticsPassActive?1:0; }
extern "C" __declspec(dllexport) int copvr_optics_ready(int eye) { return eye>=0&&eye<2&&copvrOpticsRT[eye]&&copvrOpticsFrame[eye]==Device.dwFrame; }
extern "C" __declspec(dllexport) void copvr_set_optics_pass(int active) {
    for(int eye=0;eye<2;++eye) if(!copvrOpticsRT[eye]) copvrOpticsRT[eye].create(eye?"$user$copvr_optics_right":"$user$copvr_optics_left",Device.dwWidth,Device.dwHeight,D3DFMT_A8R8G8B8);
    copvrOpticsPassActive=active!=0; if(active) { copvrOpticsEye=active-1; copvrOpticsFrame[copvrOpticsEye]=u32(-1); }
}
extern "C" __declspec(dllexport) int copvr_copy_optics(void* texture) {
    if(!texture||!copvrOpticsPassActive) return 0;
    auto& target=copvrOpticsRT[copvrOpticsEye];
    auto source=static_cast<ID3D11Texture2D*>(texture);
    D3D11_TEXTURE2D_DESC a{},b{}; source->GetDesc(&a); target->pSurface->GetDesc(&b);
    if(a.Width!=b.Width||a.Height!=b.Height||a.Format!=b.Format||a.SampleDesc.Count!=b.SampleDesc.Count) return 0;
    HW.get_context(CHW::IMM_CTX_ID)->CopyResource(target->pSurface,source);
    copvrOpticsFrame[copvrOpticsEye]=Device.dwFrame; return 1;
}
static ID3D11RenderTargetView* copvrWristSavedRT[4]{};
static ID3D11DepthStencilView* copvrWristSavedZB{};
static bool copvrOverlay{},copvrUICleared{}; static int copvrEye{};
extern "C" __declspec(dllexport) int copvr_current_eye() { return copvrEye; }
extern "C" int copvr_motion_view_index() {return copvrOpticsPassActive?2+copvrOpticsEye:copvrEye;}
extern "C" __declspec(dllexport) void copvr_set_ui(int enabled,int eye) {
    copvrOverlay=enabled!=0; copvrEye=eye; if(eye==0) copvrUICleared=false;
}
extern "C" __declspec(dllexport) void copvr_release_ui() {
    for(int eye=0;eye<2;++eye) { copvrOpticsRT[eye].destroy(); copvrOpticsFrame[eye]=u32(-1); } copvrOpticsPassActive=false;
    for(auto& rt:copvrWristRT) rt.destroy();
    if(copvrUIRT) copvrUIRT->Release(); if(copvrUITexture) copvrUITexture->Release();
    copvrUIRT=nullptr; copvrUITexture=nullptr; copvrOverlay=false;
}
extern "C" __declspec(dllexport) void* copvr_get_ui_texture() { return copvrUITexture; }
extern "C" __declspec(dllexport) int copvr_ui_begin() {
    if(!copvrOverlay) return 0;
    if(copvrEye!=0) return 2;
    if(!copvrUITexture) {
        D3D11_TEXTURE2D_DESC d{}; d.Width=Device.dwWidth; d.Height=Device.dwHeight;
        d.MipLevels=d.ArraySize=d.SampleDesc.Count=1; d.Format=DXGI_FORMAT_R8G8B8A8_UNORM;
        d.Usage=D3D11_USAGE_DEFAULT; d.BindFlags=D3D11_BIND_RENDER_TARGET;
        CHK_DX(HW.pDevice->CreateTexture2D(&d,nullptr,&copvrUITexture));
        CHK_DX(HW.pDevice->CreateRenderTargetView(copvrUITexture,nullptr,&copvrUIRT));
    }
    for(u32 i=0;i<4;++i) { copvrSavedRT[i]=RCache.get_RT(i); RCache.set_RT(i==0?copvrUIRT:nullptr,i); }
    copvrSavedZB=RCache.get_ZB(); RCache.set_ZB(nullptr);
    if(!copvrUICleared) { const float clear[4]={}; HW.get_context(CHW::IMM_CTX_ID)->ClearRenderTargetView(copvrUIRT,clear); copvrUICleared=true; }
    return 1;
}
extern "C" __declspec(dllexport) void copvr_ui_end() {
    for(u32 i=0;i<4;++i) RCache.set_RT(copvrSavedRT[i],i);
    RCache.set_ZB(copvrSavedZB);
}
extern "C" __declspec(dllexport) int copvr_wrist_begin(int side) {
    if(side<0||side>1) return 0;
    auto& rt=copvrWristRT[side];
    if(!rt) rt.create(side?"$user$copvr_wrist1":"$user$copvr_wrist0",Device.dwWidth,Device.dwHeight,D3DFMT_A8R8G8B8);
    for(int i=0;i<4;++i) { copvrWristSavedRT[i]=RCache.get_RT(i); RCache.set_RT(i==0?rt->pRT:nullptr,i); }
    copvrWristSavedZB=RCache.get_ZB(); RCache.set_ZB(nullptr);
    const float clear[4]={}; HW.get_context(CHW::IMM_CTX_ID)->ClearRenderTargetView(rt->pRT,clear);
    return 1;
}
extern "C" __declspec(dllexport) void copvr_wrist_end() {
    for(int i=0;i<4;++i) RCache.set_RT(copvrWristSavedRT[i],i);
    RCache.set_ZB(copvrWristSavedZB);
}
#endif
