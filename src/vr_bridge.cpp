#include "physical_posture.hpp"
#include "openxr_d3d11.hpp"
#include "vr_bridge_api.h"
#include "ui_ray.hpp"
#include "beam_math.hpp"
#include "ui_aim_filter.hpp"
#include "vr_ui_layout.hpp"
#include <atomic>
#include "vr_gpu_profile.hpp"
using namespace copvr;
namespace {
std::unique_ptr<Host> host;
ProfileTrace profile("bridge");
std::unique_ptr<GpuProfile> gpuProfile;
uint64_t profileFrame{};
std::string lastError;
XrTime predictedTime{}; bool begun{},render{},calibrated{};
bool menu{},overlay{},passiveUI{},uiCopied{},panelPlaced{}; XrPosef panelPose{};
std::array<XrView,2> views;
std::array<bool,2> copied{};
Vec3 origin; Quaternion inverseYaw;
uint64_t frames{};
CopVrControls controls{COPVR_CONTROLS_ABI};
CopVrGrips grips{COPVR_GRIP_ABI};
CopVrPointer pointer{COPVR_POINTER_ABI};
int uiPointerOwner=-1;
float uiPointerX=.5f,uiPointerY=.5f;
UiAimFilter uiAimFilter;
CopVrHead head{COPVR_HEAD_ABI};
CopVrHeight height{COPVR_HEIGHT_ABI};
std::atomic<bool> calibrateHeight{false};
CopVrNavigation navigation{COPVR_NAV_ABI};
struct Beam { Pose pose; float length{}; };
std::array<Beam,2> beams;
void log(const std::string& message) { OutputDebugStringA(("[CoPVR] "+message+"\n").c_str()); }
template<class F> int guarded(F f) {
    try { f(); return 1; } catch(const std::exception& e) { lastError=e.what(); log(lastError); return -1; }
}
void finish(bool submit) {
    if(!begun) return;
    std::array<XrCompositionLayerProjectionView,2> projection{{{XR_TYPE_COMPOSITION_LAYER_PROJECTION_VIEW},{XR_TYPE_COMPOSITION_LAYER_PROJECTION_VIEW}}};
    for(int i=0;i<2;++i) {
        projection[i].pose=views[i].pose; projection[i].fov=views[i].fov;
        projection[i].subImage.swapchain=host->eyes[i].swapchain;
        projection[i].subImage.imageRect.extent={static_cast<int32_t>(host->eyes[i].width),static_cast<int32_t>(host->eyes[i].height)};
    }
    XrCompositionLayerProjection layer{XR_TYPE_COMPOSITION_LAYER_PROJECTION}; layer.space=host->space;
    layer.viewCount=2; layer.views=projection.data();
    XrCompositionLayerQuad panel{XR_TYPE_COMPOSITION_LAYER_QUAD};
    panel.space=host->space; panel.eyeVisibility=XR_EYE_VISIBILITY_BOTH; panel.pose=panelPose;
    panel.subImage=projection[0].subImage;
    if(overlay) {
        panel.subImage.swapchain=host->uiOverlay.swapchain;
        panel.layerFlags=XR_COMPOSITION_LAYER_BLEND_TEXTURE_SOURCE_ALPHA_BIT;
    }
    panel.size={uiPanelWidth,uiPanelHeight};
    const auto* base=menu?reinterpret_cast<const XrCompositionLayerBaseHeader*>(&panel):
        reinterpret_cast<const XrCompositionLayerBaseHeader*>(&layer);
    std::array<const XrCompositionLayerBaseHeader*,4> layers{{base}};
    std::array<XrCompositionLayerQuad,2> beamLayers{{{XR_TYPE_COMPOSITION_LAYER_QUAD},{XR_TYPE_COMPOSITION_LAYER_QUAD}}};
    uint32_t layerCount=1;
    if(overlay&&uiCopied) layers[layerCount++]=reinterpret_cast<const XrCompositionLayerBaseHeader*>(&panel);
    if((menu||overlay)&&!passiveUI&&host->uiBeam.swapchain) for(int i=0;i<2;++i) {
        const auto& b=beams[i]; if(!b.pose.valid) continue;
        auto& quad=beamLayers[i]; quad.space=host->space; quad.eyeVisibility=XR_EYE_VISIBILITY_BOTH;
        auto p=b.pose.position; auto q=b.pose.orientation;
        quad.pose={{q.x,q.y,q.z,q.w},{p.x,p.y,p.z}};
        quad.size={.004f,b.length}; quad.subImage.swapchain=host->uiBeam.swapchain;
        quad.subImage.imageRect.extent={8,8};
        layers[layerCount++]=reinterpret_cast<const XrCompositionLayerBaseHeader*>(&quad);
    }
    XrFrameEndInfo end{XR_TYPE_FRAME_END_INFO}; end.displayTime=predictedTime; end.environmentBlendMode=XR_ENVIRONMENT_BLEND_MODE_OPAQUE;
    end.layerCount=submit?layerCount:0; end.layers=submit?layers.data():nullptr;
    begun=false;
    if(gpuProfile)gpuProfile->end(host->context.p);
    { ProfileSpan timing(profile,"xr_end",profileFrame);
    xr(host->fn<PFN_xrEndFrame>("xrEndFrame")(host->session,&end),"bridge xrEndFrame"); }
    if(submit) ++frames;
}
}
extern "C" __declspec(dllexport) int copvr_initialize(void* device,uint32_t width,uint32_t height) {
    return guarded([&] {
        if(!device||!width||!height) throw std::runtime_error("Engine device and render dimensions are required");
        if(host) throw std::runtime_error("VR bridge already initialized");
        auto pending=std::make_unique<Host>();
        if(!pending->initialize(false)) throw std::runtime_error("Quest Link headset unavailable");
        pending->initializeGraphics(static_cast<ID3D11Device*>(device),width,height);
        pending->initializeBeam();
        pending->initializeOverlay();
        if(profile.enabled())gpuProfile=std::make_unique<GpuProfile>();
        host=std::move(pending); calibrated=false; menu=panelPlaced=false; frames=0; lastError.clear();
        uiAimFilter.reset(); uiPointerOwner=-1;
    });
}
extern "C" __declspec(dllexport) int copvr_begin(CopVrFrame* output) {
    int state=0;
    auto result=guarded([&] {
        if(!host||!output||output->abi!=COPVR_ABI) throw std::runtime_error("Invalid bridge/frame ABI");
        if(begun) throw std::runtime_error("Previous VR frame has not ended");
        output->render=output->focused=0; controls.active=0; grips.active=0; pointer.active=0; head.active=0; navigation.active=0; beams={};
        menu=output->menu==1; passiveUI=output->menu==3; overlay=output->menu==2||passiveUI; uiCopied=false;
        if(!menu&&!overlay) panelPlaced=false;
        host->events(); if(host->exit) { uiAimFilter.reset(); state=-2; return; }
        if(!host->running) { uiAimFilter.reset(); return; }
        ++profileFrame;
        XrFrameWaitInfo wait{XR_TYPE_FRAME_WAIT_INFO}; XrFrameState frame{XR_TYPE_FRAME_STATE};
        { ProfileSpan timing(profile,"xr_wait",profileFrame);
        xr(host->fn<PFN_xrWaitFrame>("xrWaitFrame")(host->session,&wait,&frame),"bridge xrWaitFrame"); }
        if(profile.enabled())profile.record("xr_period",profileFrame,profileClockMs(),double(frame.predictedDisplayPeriod)/1000000.0);
        XrFrameBeginInfo begin{XR_TYPE_FRAME_BEGIN_INFO};
        { ProfileSpan timing(profile,"xr_begin",profileFrame);
        xr(host->fn<PFN_xrBeginFrame>("xrBeginFrame")(host->session,&begin),"bridge xrBeginFrame"); }
        if(gpuProfile&&frame.shouldRender)gpuProfile->begin(host->device.p,host->context.p,profile,profileFrame);
        begun=true; predictedTime=frame.predictedDisplayTime; copied={false,false};
        auto input=host->input->sample(predictedTime,host->focused);
        if((menu||overlay)&&(!panelPlaced||passiveUI)&&input.head.valid) {
            const auto forward=rotate(input.head.orientation,{0,0,-1});
            float yaw=std::atan2(-forward.x,-forward.z);
            Quaternion q{0,std::sin(yaw*.5f),0,std::cos(yaw*.5f)};
            Vec3 p=input.head.position+rotate(q,{0,0,-uiPanelDistance});
            panelPose={{q.x,q.y,q.z,q.w},{p.x,p.y,p.z}}; panelPlaced=true;
        }
        output->focused=host->focused;
        XrViewLocateInfo locate{XR_TYPE_VIEW_LOCATE_INFO}; locate.viewConfigurationType=XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;
        locate.displayTime=predictedTime; locate.space=host->space;
        XrViewState viewState{XR_TYPE_VIEW_STATE}; uint32_t count{};
        views={XrView{XR_TYPE_VIEW},XrView{XR_TYPE_VIEW}};
        xr(host->fn<PFN_xrLocateViews>("xrLocateViews")(host->session,&locate,&viewState,2,&count,views.data()),"bridge xrLocateViews");
        constexpr auto valid=XR_VIEW_STATE_POSITION_VALID_BIT|XR_VIEW_STATE_ORIENTATION_VALID_BIT;
        render=frame.shouldRender&&count==2&&(viewState.viewStateFlags&valid)==valid&&(!(menu||overlay)||panelPlaced);
        const bool firstCalibration=!calibrated&&input.head.valid;
        const bool requested=calibrateHeight.exchange(false);
        if(input.head.valid&&(firstCalibration||requested)) {
            uiAimFilter.reset();
            origin=input.head.position;
            Vec3 forward=rotate(input.head.orientation,{0,0,-1});
            float yaw=std::atan2(-forward.x,-forward.z);
            inverseYaw={0,-std::sin(yaw*.5f),0,std::cos(yaw*.5f)};calibrated=true;
            height.standingEyeHeight=input.floorValid?input.floorEyeHeight:1.65f;
            height.floorValid=input.floorValid?1u:0u;++height.serial;
            log("Full VR origin, heading and standing height calibrated: "+std::to_string(height.standingEyeHeight));
        }
        height.active=render&&input.head.valid&&host->focused;
        height.timestampMs=GetTickCount64();
        if(!calibrated) render=false;
        navigation={}; navigation.abi=COPVR_NAV_ABI; navigation.timestampMs=GetTickCount64();
        navigation.active=render&&input.head.valid&&host->focused;
        if(navigation.active) {
            const auto& l=input.hands[0]; const auto& r=input.hands[1];
            {
                if(l.grip.valid&&l.aim.valid) navigation.tracked|=1;
                navigation.moveX=l.stickX; navigation.moveY=l.stickY;
                navigation.buttons|=(l.buttons&1?COPVR_X:0)|(l.buttons&2?COPVR_Y:0)|(l.buttons&4?COPVR_LSTICK:0)|(l.buttons&8?COPVR_MENU:0)|(l.trigger>=.75f?COPVR_USE:0);
            }
            {
                if(r.grip.valid&&r.aim.valid) navigation.tracked|=2;
                navigation.turnX=r.stickX; navigation.scrollY=r.stickY;
                navigation.buttons|=(r.buttons&1?COPVR_A:0)|(r.buttons&2?COPVR_B:0)|(r.buttons&4?COPVR_RSTICK:0);
                if(!menu&&(!overlay||passiveUI)) navigation.buttons|=(r.stickY<-.75f?COPVR_HOLSTER:0)|(r.stickY>.75f?COPVR_RELOAD:0);
            }
        }
        head={}; head.abi=COPVR_HEAD_ABI; head.timestampMs=GetTickCount64();
        if(render&&input.head.valid&&host->focused) {
            auto p=to_xray(rotate(inverseYaw,input.head.position-origin));
            auto d=to_xray(rotate(inverseYaw,rotate(input.head.orientation,{0,0,-1})));
            auto u=to_xray(rotate(inverseYaw,rotate(input.head.orientation,{0,1,0})));
            head={COPVR_HEAD_ABI,1,GetTickCount64(),{p.x,p.y,p.z},{d.x,d.y,d.z},{u.x,u.y,u.z}};
        }
        pointer={}; pointer.abi=COPVR_POINTER_ABI; pointer.timestampMs=GetTickCount64();
        uiAimFilter.context((menu||overlay)&&!passiveUI&&render&&host->focused?(menu?1u:2u):0u);
        if((menu||overlay)&&!passiveUI&&render&&host->focused) {
            // An intentional left grip selects its ray for physical extraction;
            // normal menu pointing retains the previously validated right preference.
            if(uiPointerOwner>=0&&!input.hands[uiPointerOwner].aim.valid) uiPointerOwner=-1;
            const int preferred=uiPointerOwner>=0?uiPointerOwner:
                input.hands[0].trigger>.7f&&input.hands[1].trigger<.7f?0:
                input.hands[0].squeeze>.7f&&input.hands[1].squeeze<.7f?0:1;
            for(int i : {preferred,1-preferred}) {
                const auto& h=input.hands[i];
                const auto aim=uiAimFilter.update(i,h.aim,pointer.timestampMs);
                if(!aim.valid) continue;
                auto direction=rotate(aim.orientation,{0,0,-1});
                auto hit=panelHit(aim.position,direction,
                    {panelPose.position.x,panelPose.position.y,panelPose.position.z},
                    {panelPose.orientation.x,panelPose.orientation.y,panelPose.orientation.z,panelPose.orientation.w},
                    uiPanelWidth,uiPanelHeight);
                float length=hit.valid?std::max(0.f,hit.distance-.01f):2.5f;
                if(input.head.valid&&length>.05f) {
                    auto start=aim.position+direction*.03f,end=aim.position+direction*length;
                    beams[i]={beamPose(start,end,input.head.position),(end-start).length()};
                }
                if(hit.valid&&!pointer.active&&(uiPointerOwner<0||uiPointerOwner==i)) {
                    pointer.active=1; pointer.x=hit.x; pointer.y=hit.y; pointer.trigger=h.trigger; pointer.hand=i;
                    uiPointerX=hit.x; uiPointerY=hit.y;
                    if(h.trigger>.7f) uiPointerOwner=i;
                }
            }
            if(!pointer.active&&uiPointerOwner>=0) {
                pointer.active=1; pointer.x=uiPointerX; pointer.y=uiPointerY;
                pointer.trigger=input.hands[uiPointerOwner].trigger; pointer.hand=uiPointerOwner;
            }
            if(uiPointerOwner>=0&&input.hands[uiPointerOwner].trigger<.25f) uiPointerOwner=-1;
        } else uiPointerOwner=-1;
        controls={}; controls.abi=COPVR_CONTROLS_ABI; controls.timestampMs=GetTickCount64();
        controls.active=render&&input.head.valid&&host->focused&&!menu&&(!overlay||passiveUI);
        grips={}; grips.abi=COPVR_GRIP_ABI; grips.active=controls.active; grips.timestampMs=controls.timestampMs;
        if(render&&input.head.valid&&host->focused) for(int i=0;i<2;++i) {
            const auto& h=input.hands[i]; auto& out=controls.hands[i];
            out.valid=h.grip.valid&&h.aim.valid;
            if(!out.valid) continue;
            auto p=to_xray(rotate(inverseYaw,h.aim.position-origin));
            auto g=to_xray(rotate(inverseYaw,h.grip.position-origin));
            auto d=to_xray(rotate(inverseYaw,rotate(h.aim.orientation,{0,0,-1})));
            auto u=to_xray(rotate(inverseYaw,rotate(h.aim.orientation,{0,1,0})));
            out={1,h.trigger,h.squeeze,{p.x,p.y,p.z},{d.x,d.y,d.z},{u.x,u.y,u.z},{g.x,g.y,g.z}};
            auto gd=to_xray(rotate(inverseYaw,rotate(h.grip.orientation,{0,0,-1})));
            auto gu=to_xray(rotate(inverseYaw,rotate(h.grip.orientation,{0,1,0})));
            grips.hands[i]={1,{gd.x,gd.y,gd.z},{gu.x,gu.y,gu.z}};
        }
        if(!render) { finish(false); return; }
        for(int i=0;i<2;++i) {
            const auto& v=views[i]; auto& e=output->eyes[i];
            Vec3 p=to_xray(rotate(inverseYaw,Vec3{v.pose.position.x,v.pose.position.y,v.pose.position.z}-origin));
            Quaternion q{v.pose.orientation.x,v.pose.orientation.y,v.pose.orientation.z,v.pose.orientation.w};
            Vec3 d=to_xray(rotate(inverseYaw,rotate(q,{0,0,-1}))),u=to_xray(rotate(inverseYaw,rotate(q,{0,1,0})));
            e={{p.x,p.y,p.z},{d.x,d.y,d.z},{u.x,u.y,u.z},v.fov.angleLeft,v.fov.angleRight,v.fov.angleUp,v.fov.angleDown};
        }
        output->render=1; state=1;
    });
    return result<0?result:state;
}
extern "C" __declspec(dllexport) int copvr_copy_eye(uint32_t eye,void* texture) {
    return guarded([&] {
        if(!host||!begun||!render||eye>1||!texture||copied[eye]) throw std::runtime_error("Invalid eye copy state");
        ProfileSpan copyTiming(profile,eye==0?"eye0_copy_cpu":"eye1_copy_cpu",profileFrame);
        auto& target=host->eyes[eye]; auto* source=static_cast<ID3D11Texture2D*>(texture);
        Com<ID3D11Device> sourceDevice; source->GetDevice(sourceDevice.put());
        if(sourceDevice.p!=host->device.p) throw std::runtime_error("Backbuffer belongs to a different D3D11 device");
        D3D11_TEXTURE2D_DESC desc{}; source->GetDesc(&desc);
        if(desc.Width!=target.width||desc.Height!=target.height||desc.SampleDesc.Count!=1||desc.ArraySize!=1||desc.MipLevels!=1)
            throw std::runtime_error("Game backbuffer resized; restart VR with a stable video mode");
        if(desc.Format!=DXGI_FORMAT_R8G8B8A8_UNORM&&desc.Format!=DXGI_FORMAT_R8G8B8A8_TYPELESS)
            throw std::runtime_error("Game backbuffer must be RGBA8 for the VR copy path");
        if(target.format!=DXGI_FORMAT_R8G8B8A8_UNORM) throw std::runtime_error("Runtime swapchain must support RGBA8 for the VR copy path");
        uint32_t index{}; XrSwapchainImageAcquireInfo acquire{XR_TYPE_SWAPCHAIN_IMAGE_ACQUIRE_INFO};
        xr(host->fn<PFN_xrAcquireSwapchainImage>("xrAcquireSwapchainImage")(target.swapchain,&acquire,&index),"bridge acquire");
        XrSwapchainImageWaitInfo wait{XR_TYPE_SWAPCHAIN_IMAGE_WAIT_INFO}; wait.timeout=XR_INFINITE_DURATION;
        { ProfileSpan timing(profile,"swapchain_eye_wait",profileFrame);
        xr(host->fn<PFN_xrWaitSwapchainImage>("xrWaitSwapchainImage")(target.swapchain,&wait),"bridge wait image"); }
        host->context->CopyResource(target.images.at(index).texture,source);
        XrSwapchainImageReleaseInfo release{XR_TYPE_SWAPCHAIN_IMAGE_RELEASE_INFO};
        xr(host->fn<PFN_xrReleaseSwapchainImage>("xrReleaseSwapchainImage")(target.swapchain,&release),"bridge release image");
        copied[eye]=true;
    });
}
extern "C" __declspec(dllexport) int copvr_end() {
    return guarded([&] {
        finish(render&&copied[0]&&(menu||copied[1]));
    });
}
extern "C" __declspec(dllexport) void copvr_profile_stage(const char* name,int opening) {
    if(profile.enabled()&&host&&gpuProfile&&name)
        gpuProfile->marker(host->device.p,host->context.p,name,opening!=0);
}
extern "C" __declspec(dllexport) int copvr_copy_ui(void* texture) {
    return guarded([&] {
        if(!host||!begun||!render||!overlay||!texture||uiCopied) throw std::runtime_error("Invalid UI copy state");
        auto* source=static_cast<ID3D11Texture2D*>(texture); auto& target=host->uiOverlay;
        D3D11_TEXTURE2D_DESC desc{}; source->GetDesc(&desc);
        Com<ID3D11Device> device; source->GetDevice(device.put());
        if(device.p!=host->device.p||desc.Width!=target.width||desc.Height!=target.height||
            desc.Format!=target.format||desc.SampleDesc.Count!=1||desc.ArraySize!=1||desc.MipLevels!=1)
            throw std::runtime_error("Incompatible UI texture");
        uint32_t index{}; XrSwapchainImageAcquireInfo acquire{XR_TYPE_SWAPCHAIN_IMAGE_ACQUIRE_INFO};
        xr(host->fn<PFN_xrAcquireSwapchainImage>("xrAcquireSwapchainImage")(target.swapchain,&acquire,&index),"UI acquire");
        XrSwapchainImageWaitInfo wait{XR_TYPE_SWAPCHAIN_IMAGE_WAIT_INFO}; wait.timeout=XR_INFINITE_DURATION;
        { ProfileSpan timing(profile,"swapchain_ui_wait",profileFrame);
        xr(host->fn<PFN_xrWaitSwapchainImage>("xrWaitSwapchainImage")(target.swapchain,&wait),"UI wait"); }
        host->context->CopyResource(target.images.at(index).texture,source);
        XrSwapchainImageReleaseInfo release{XR_TYPE_SWAPCHAIN_IMAGE_RELEASE_INFO};
        xr(host->fn<PFN_xrReleaseSwapchainImage>("xrReleaseSwapchainImage")(target.swapchain,&release),"UI release");
        uiCopied=true;
    });
}
extern "C" __declspec(dllexport) const char* copvr_error() { return lastError.c_str(); }
extern "C" __declspec(dllexport) int copvr_get_controls(CopVrControls* output) {
    if(!output||output->abi!=COPVR_CONTROLS_ABI) return -1;
    *output=controls;
    if(!host||!host->focused||GetTickCount64()-controls.timestampMs>100) output->active=0;
    return output->active?1:0;
}
extern "C" __declspec(dllexport) int copvr_get_grips(CopVrGrips* output) {
    if(!output||output->abi!=COPVR_GRIP_ABI) return -1;
    *output=grips;
    if(!host||!host->focused||GetTickCount64()-grips.timestampMs>100) output->active=0;
    return output->active?1:0;
}
extern "C" __declspec(dllexport) int copvr_get_ui_controls(CopVrControls* output) {
    if(!output||output->abi!=COPVR_CONTROLS_ABI) return -1;
    *output=controls;
    output->active=host&&host->focused&&overlay&&!passiveUI&&render&&GetTickCount64()-controls.timestampMs<=100;
    return output->active?1:0;
}
extern "C" __declspec(dllexport) int copvr_pulse(uint32_t hand,float amplitude,uint32_t milliseconds) {
    if(!host||!host->focused) return 0;
    return guarded([&] { host->input->pulse(static_cast<int>(hand),amplitude,milliseconds); });
}
extern "C" __declspec(dllexport) int copvr_get_pointer(CopVrPointer* output) {
    if(!output||output->abi!=COPVR_POINTER_ABI) return -1;
    *output=pointer;
    if(!host||!host->focused||GetTickCount64()-pointer.timestampMs>100) output->active=0;
    return output->active?1:0;
}
extern "C" __declspec(dllexport) int copvr_get_head(CopVrHead* output) {
    if(!output||output->abi!=COPVR_HEAD_ABI) return -1;
    *output=head;
    if(!host||!host->focused||GetTickCount64()-head.timestampMs>100) output->active=0;
    return output->active?1:0;
}
extern "C" __declspec(dllexport) int copvr_get_height(CopVrHeight* output) {
    if(!output||output->abi!=COPVR_HEIGHT_ABI) return -1;
    *output=height;
    if(!host||!host->focused||GetTickCount64()-height.timestampMs>100) output->active=0;
    return output->active?1:0;
}
extern "C" __declspec(dllexport) int copvr_calibrate_height() {
    if(!host||!host->focused||!head.active||!copvr::canCalibrateStanding(head.position[1])||GetTickCount64()-head.timestampMs>100)return 0;
    calibrateHeight.store(true); return 1;
}
extern "C" __declspec(dllexport) int copvr_get_navigation(CopVrNavigation* output) {
    if(!output||output->abi!=COPVR_NAV_ABI) return -1;
    *output=navigation;
    if(!host||!host->focused||GetTickCount64()-navigation.timestampMs>100) output->active=0;
    return output->active?1:0;
}
extern "C" __declspec(dllexport) void copvr_shutdown() {
    if(host) {
        guarded([] { finish(false); });
        log("Submitted engine stereo frames: "+std::to_string(frames)); gpuProfile.reset(); host.reset();
    }
    calibrated=begun=render=false;
    uiAimFilter.reset(); uiPointerOwner=-1;
    calibrateHeight.store(false); height={}; height.abi=COPVR_HEIGHT_ABI;
    controls={}; controls.abi=COPVR_CONTROLS_ABI;
    grips={}; grips.abi=COPVR_GRIP_ABI;
}
