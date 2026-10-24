#pragma once
// OpenXR/D3D11 host. Supports standalone device creation or an engine-owned device.
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <d3d11.h>
#include <dxgi1_2.h>
#include <dxgi1_4.h>
#include <d3dcompiler.h>
#define XR_USE_PLATFORM_WIN32
#define XR_USE_GRAPHICS_API_D3D11
#include "openxr_input.hpp"
#include <openxr/openxr_platform.h>
#include "stereo_math.hpp"
#include <algorithm>
#include <chrono>
#include <cstring>
#include <iostream>
#include <memory>
#include <string>

namespace copvr {
template<class T> struct Com {
    T* p{};
    ~Com() { if(p) p->Release(); }
    Com()=default; Com(const Com&)=delete; Com& operator=(const Com&)=delete;
    T** put() { if(p) p->Release(); p=nullptr; return &p; }
    T* operator->() const { return p; }
};
inline void hr(HRESULT result,const char* name) { if(FAILED(result)) throw std::runtime_error(std::string(name)+" HRESULT="+std::to_string(result)); }
inline void xr(XrResult result,const char* name) { if(XR_FAILED(result)) throw std::runtime_error(std::string(name)+" XrResult="+std::to_string(result)); }
struct Loader {
    HMODULE module{}; PFN_xrGetInstanceProcAddr get{};
    Loader() {
        wchar_t executable[32768]{};
        if(!GetModuleFileNameW(nullptr,executable,32768)) throw std::runtime_error("GetModuleFileNameW");
        std::wstring path(executable); path.resize(path.find_last_of(L"\\/")+1); path+=L"openxr_loader.dll";
        module=LoadLibraryExW(path.c_str(),nullptr,LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR|LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
        if(!module) throw std::runtime_error("Cannot load app-local openxr_loader.dll, Win32="+std::to_string(GetLastError()));
        get=reinterpret_cast<PFN_xrGetInstanceProcAddr>(GetProcAddress(module,"xrGetInstanceProcAddr"));
        if(!get) { FreeLibrary(module); module=nullptr; throw std::runtime_error("Missing xrGetInstanceProcAddr"); }
    }
    ~Loader() { if(module) FreeLibrary(module); }
    template<class T> T fn(XrInstance instance,const char* name) {
        PFN_xrVoidFunction result{}; xr(get(instance,name,&result),name); return reinterpret_cast<T>(result);
    }
};
struct Eye {
    XrSwapchain swapchain{}; uint32_t width{},height{};
    DXGI_FORMAT format{};
    std::vector<XrSwapchainImageD3D11KHR> images;
};
struct Host {
    Loader loader; XrInstance instance{}; XrSystemId system{}; XrSession session{}; XrSpace space{};
    Com<ID3D11Device> device; Com<ID3D11DeviceContext> context;
    std::array<Eye,2> eyes;
    Eye uiBeam;
    Eye uiOverlay;
    std::unique_ptr<OpenXRInput> input;
    bool running{},focused{},exit{};
    XrSessionState state{XR_SESSION_STATE_UNKNOWN};
    template<class T> T fn(const char* name) { return loader.fn<T>(instance,name); }
    Host()=default;
    ~Host() {
        // No throwing API lookup in teardown, including partial initialization.
        input.reset();
        auto cleanup=[&](const char* name,auto handle) {
            if(!handle) return;
            PFN_xrVoidFunction p{}; loader.get(instance,name,&p);
            if(p) {
                using Handle=decltype(handle);
                reinterpret_cast<XrResult(XRAPI_PTR*)(Handle)>(p)(handle);
            }
        };
        if(context.p) { context->ClearState(); context->Flush(); }
        for(auto& e:eyes) cleanup("xrDestroySwapchain",e.swapchain);
        cleanup("xrDestroySwapchain",uiBeam.swapchain);
        cleanup("xrDestroySwapchain",uiOverlay.swapchain);
        cleanup("xrDestroySpace",space);
        cleanup("xrDestroySession",session);
        cleanup("xrDestroyInstance",instance);
    }
    bool initialize(bool graphics) {
        if(instance) throw std::runtime_error("OpenXR instance already initialized");
        uint32_t count{};
        auto enumerate=loader.fn<PFN_xrEnumerateInstanceExtensionProperties>(XR_NULL_HANDLE,"xrEnumerateInstanceExtensionProperties");
        xr(enumerate(nullptr,0,&count,nullptr),"extension count");
        std::vector<XrExtensionProperties> extensions(count,{XR_TYPE_EXTENSION_PROPERTIES});
        xr(enumerate(nullptr,count,&count,extensions.data()),"extensions");
        bool d3d=false;
        for(const auto& e:extensions) if(std::strcmp(e.extensionName,XR_KHR_D3D11_ENABLE_EXTENSION_NAME)==0) d3d=true;
        if(!d3d) throw std::runtime_error("Runtime lacks XR_KHR_D3D11_enable");
        const char* enabled=XR_KHR_D3D11_ENABLE_EXTENSION_NAME;
        XrInstanceCreateInfo info{XR_TYPE_INSTANCE_CREATE_INFO};
        std::strncpy(info.applicationInfo.applicationName,"CoP VR diagnostic",XR_MAX_APPLICATION_NAME_SIZE-1);
        info.applicationInfo.applicationVersion=1; info.applicationInfo.apiVersion=XR_MAKE_VERSION(1,0,0);
        info.enabledExtensionCount=1; info.enabledExtensionNames=&enabled;
        xr(loader.fn<PFN_xrCreateInstance>(XR_NULL_HANDLE,"xrCreateInstance")(&info,&instance),"xrCreateInstance");
        XrInstanceProperties properties{XR_TYPE_INSTANCE_PROPERTIES};
        xr(fn<PFN_xrGetInstanceProperties>("xrGetInstanceProperties")(instance,&properties),"runtime properties");
        std::cout<<"Runtime: "<<properties.runtimeName<<'\n';
        XrSystemGetInfo query{XR_TYPE_SYSTEM_GET_INFO}; query.formFactor=XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY;
        auto result=fn<PFN_xrGetSystem>("xrGetSystem")(instance,&query,&system);
        if(result==XR_ERROR_FORM_FACTOR_UNAVAILABLE) { std::cout<<"HEADSET_UNAVAILABLE: connect Quest Link and enter PC VR.\n"; return false; }
        xr(result,"xrGetSystem");
        XrSystemProperties systemProperties{XR_TYPE_SYSTEM_PROPERTIES};
        xr(fn<PFN_xrGetSystemProperties>("xrGetSystemProperties")(instance,system,&systemProperties),"system properties");
        std::cout<<"System: "<<systemProperties.systemName<<"; orientation="<<systemProperties.trackingProperties.orientationTracking
            <<" position="<<systemProperties.trackingProperties.positionTracking<<'\n';
        if(!graphics) return true;
        return initializeGraphics();
    }
    void initializeBeam() {
        auto& e=uiBeam; e.width=e.height=8; e.format=eyes[0].format;
        XrSwapchainCreateInfo info{XR_TYPE_SWAPCHAIN_CREATE_INFO};
        info.createFlags=XR_SWAPCHAIN_CREATE_STATIC_IMAGE_BIT;
        info.usageFlags=XR_SWAPCHAIN_USAGE_COLOR_ATTACHMENT_BIT;
        info.format=e.format; info.width=e.width; info.height=e.height;
        info.sampleCount=info.faceCount=info.arraySize=info.mipCount=1;
        xr(fn<PFN_xrCreateSwapchain>("xrCreateSwapchain")(session,&info,&e.swapchain),"beam swapchain");
        uint32_t count{};
        auto images=fn<PFN_xrEnumerateSwapchainImages>("xrEnumerateSwapchainImages");
        xr(images(e.swapchain,0,&count,nullptr),"beam image count");
        e.images.resize(count,{XR_TYPE_SWAPCHAIN_IMAGE_D3D11_KHR});
        xr(images(e.swapchain,count,&count,reinterpret_cast<XrSwapchainImageBaseHeader*>(e.images.data())),"beam images");
        uint32_t index{}; XrSwapchainImageAcquireInfo acquire{XR_TYPE_SWAPCHAIN_IMAGE_ACQUIRE_INFO};
        xr(fn<PFN_xrAcquireSwapchainImage>("xrAcquireSwapchainImage")(e.swapchain,&acquire,&index),"beam acquire");
        XrSwapchainImageWaitInfo wait{XR_TYPE_SWAPCHAIN_IMAGE_WAIT_INFO}; wait.timeout=XR_INFINITE_DURATION;
        xr(fn<PFN_xrWaitSwapchainImage>("xrWaitSwapchainImage")(e.swapchain,&wait),"beam wait");
        Com<ID3D11RenderTargetView> target;
        D3D11_RENDER_TARGET_VIEW_DESC desc{}; desc.Format=e.format; desc.ViewDimension=D3D11_RTV_DIMENSION_TEXTURE2D;
        hr(device->CreateRenderTargetView(e.images.at(index).texture,&desc,target.put()),"beam RTV");
        const float cyan[]={.05f,.85f,1.f,1.f}; context->ClearRenderTargetView(target.p,cyan);
        XrSwapchainImageReleaseInfo release{XR_TYPE_SWAPCHAIN_IMAGE_RELEASE_INFO};
        xr(fn<PFN_xrReleaseSwapchainImage>("xrReleaseSwapchainImage")(e.swapchain,&release),"beam release");
    }
    void initializeOverlay() {
        auto& e=uiOverlay; e.width=eyes[0].width; e.height=eyes[0].height; e.format=eyes[0].format;
        XrSwapchainCreateInfo info{XR_TYPE_SWAPCHAIN_CREATE_INFO}; info.usageFlags=XR_SWAPCHAIN_USAGE_TRANSFER_DST_BIT;
        info.format=e.format; info.width=e.width; info.height=e.height;
        info.sampleCount=info.faceCount=info.arraySize=info.mipCount=1;
        xr(fn<PFN_xrCreateSwapchain>("xrCreateSwapchain")(session,&info,&e.swapchain),"UI overlay swapchain");
        uint32_t count{}; auto enumerate=fn<PFN_xrEnumerateSwapchainImages>("xrEnumerateSwapchainImages");
        xr(enumerate(e.swapchain,0,&count,nullptr),"UI image count");
        e.images.resize(count,{XR_TYPE_SWAPCHAIN_IMAGE_D3D11_KHR});
        xr(enumerate(e.swapchain,count,&count,reinterpret_cast<XrSwapchainImageBaseHeader*>(e.images.data())),"UI images");
    }
    XrGraphicsRequirementsD3D11KHR graphicsRequirements() {
        if(!instance||!system) throw std::runtime_error("Initialize instance/system before querying graphics requirements");
        XrGraphicsRequirementsD3D11KHR requirements{XR_TYPE_GRAPHICS_REQUIREMENTS_D3D11_KHR};
        xr(fn<PFN_xrGetD3D11GraphicsRequirementsKHR>("xrGetD3D11GraphicsRequirementsKHR")(instance,system,&requirements),"D3D11 requirements");
        return requirements;
    }
    bool initializeGraphics(ID3D11Device* engineDevice=nullptr,uint32_t renderWidth=0,uint32_t renderHeight=0) {
        if(session||device.p) throw std::runtime_error("Graphics already initialized");
        const auto requirements=graphicsRequirements();
        if(engineDevice) {
            Com<IDXGIDevice> dxgiDevice; Com<IDXGIAdapter> engineAdapter;
            hr(engineDevice->QueryInterface(__uuidof(IDXGIDevice),reinterpret_cast<void**>(dxgiDevice.put())),"engine DXGI device");
            hr(dxgiDevice->GetAdapter(engineAdapter.put()),"engine GPU adapter");
            DXGI_ADAPTER_DESC desc{}; hr(engineAdapter->GetDesc(&desc),"engine GPU description");
            if(desc.AdapterLuid.HighPart!=requirements.adapterLuid.HighPart||desc.AdapterLuid.LowPart!=requirements.adapterLuid.LowPart)
                throw std::runtime_error("Engine GPU does not match the OpenXR runtime GPU");
            if(engineDevice->GetFeatureLevel()<requirements.minFeatureLevel) throw std::runtime_error("Engine feature level below OpenXR requirement");
            device.p=engineDevice; device->AddRef(); device->GetImmediateContext(context.put());
        } else {
        Com<IDXGIFactory1> factory; hr(CreateDXGIFactory1(__uuidof(IDXGIFactory1),reinterpret_cast<void**>(factory.put())),"DXGI factory");
        Com<IDXGIAdapter1> adapter;
        for(UINT i=0;;++i) {
            auto status=factory->EnumAdapters1(i,adapter.put());
            if(status==DXGI_ERROR_NOT_FOUND) throw std::runtime_error("OpenXR GPU adapter was not found");
            hr(status,"EnumAdapters1"); DXGI_ADAPTER_DESC1 description{}; hr(adapter->GetDesc1(&description),"adapter description");
            if(description.AdapterLuid.HighPart==requirements.adapterLuid.HighPart&&description.AdapterLuid.LowPart==requirements.adapterLuid.LowPart) break;
        }
        D3D_FEATURE_LEVEL levels[]={D3D_FEATURE_LEVEL_11_1,D3D_FEATURE_LEVEL_11_0};
        D3D_FEATURE_LEVEL selected{};
        auto status=D3D11CreateDevice(adapter.p,D3D_DRIVER_TYPE_UNKNOWN,nullptr,D3D11_CREATE_DEVICE_BGRA_SUPPORT,
            levels,2,D3D11_SDK_VERSION,device.put(),&selected,context.put());
        if(status==E_INVALIDARG) status=D3D11CreateDevice(adapter.p,D3D_DRIVER_TYPE_UNKNOWN,nullptr,D3D11_CREATE_DEVICE_BGRA_SUPPORT,
            levels+1,1,D3D11_SDK_VERSION,device.put(),&selected,context.put());
        hr(status,"D3D11CreateDevice");
        if(selected<requirements.minFeatureLevel) throw std::runtime_error("GPU feature level below runtime requirement");
        }
        XrGraphicsBindingD3D11KHR binding{XR_TYPE_GRAPHICS_BINDING_D3D11_KHR}; binding.device=device.p;
        XrSessionCreateInfo sessionInfo{XR_TYPE_SESSION_CREATE_INFO}; sessionInfo.next=&binding; sessionInfo.systemId=system;
        xr(fn<PFN_xrCreateSession>("xrCreateSession")(instance,&sessionInfo,&session),"xrCreateSession");
        XrReferenceSpaceCreateInfo reference{XR_TYPE_REFERENCE_SPACE_CREATE_INFO};
        reference.referenceSpaceType=XR_REFERENCE_SPACE_TYPE_LOCAL; reference.poseInReferenceSpace.orientation.w=1;
        xr(fn<PFN_xrCreateReferenceSpace>("xrCreateReferenceSpace")(session,&reference,&space),"local space");
        input=std::make_unique<OpenXRInput>(loader.get,instance,session,space);
        uint32_t formatCount{};
        auto formatsFn=fn<PFN_xrEnumerateSwapchainFormats>("xrEnumerateSwapchainFormats");
        xr(formatsFn(session,0,&formatCount,nullptr),"format count"); std::vector<int64_t> formats(formatCount);
        xr(formatsFn(session,formatCount,&formatCount,formats.data()),"formats");
        int64_t format=0;
        for(auto preferred:{DXGI_FORMAT_R8G8B8A8_UNORM,DXGI_FORMAT_B8G8R8A8_UNORM})
            if(std::find(formats.begin(),formats.end(),preferred)!=formats.end()) { format=preferred; break; }
        if(!format) throw std::runtime_error("No supported UNORM color format");
        uint32_t viewCount{}; auto viewsFn=fn<PFN_xrEnumerateViewConfigurationViews>("xrEnumerateViewConfigurationViews");
        xr(viewsFn(instance,system,XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO,0,&viewCount,nullptr),"view count");
        if(viewCount!=2) throw std::runtime_error("Expected exactly two stereo views");
        std::array<XrViewConfigurationView,2> views{{{XR_TYPE_VIEW_CONFIGURATION_VIEW},{XR_TYPE_VIEW_CONFIGURATION_VIEW}}};
        xr(viewsFn(instance,system,XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO,2,&viewCount,views.data()),"view dimensions");
        for(int i=0;i<2;++i) {
            auto& eye=eyes[i]; eye.width=views[i].recommendedImageRectWidth; eye.height=views[i].recommendedImageRectHeight;
            if(renderWidth&&renderHeight) {
                if(renderWidth>views[i].maxImageRectWidth||renderHeight>views[i].maxImageRectHeight)
                    throw std::runtime_error("Engine render resolution exceeds OpenXR swapchain limits");
                eye.width=renderWidth; eye.height=renderHeight;
            }
            eye.format=static_cast<DXGI_FORMAT>(format);
            XrSwapchainCreateInfo swap{XR_TYPE_SWAPCHAIN_CREATE_INFO}; swap.usageFlags=XR_SWAPCHAIN_USAGE_COLOR_ATTACHMENT_BIT|XR_SWAPCHAIN_USAGE_TRANSFER_DST_BIT;
            swap.format=format; swap.sampleCount=1; swap.width=eye.width; swap.height=eye.height; swap.faceCount=swap.arraySize=swap.mipCount=1;
            xr(fn<PFN_xrCreateSwapchain>("xrCreateSwapchain")(session,&swap,&eye.swapchain),"create eye swapchain");
            uint32_t images{}; auto imageFn=fn<PFN_xrEnumerateSwapchainImages>("xrEnumerateSwapchainImages");
            xr(imageFn(eye.swapchain,0,&images,nullptr),"image count");
            eye.images.resize(images,{XR_TYPE_SWAPCHAIN_IMAGE_D3D11_KHR});
            xr(imageFn(eye.swapchain,images,&images,reinterpret_cast<XrSwapchainImageBaseHeader*>(eye.images.data())),"swapchain images");
            std::cout<<"Eye "<<i<<": "<<eye.width<<'x'<<eye.height<<", "<<images<<" images\n";
        }
        return true;
    }
    void events() {
        for(;;) {
            XrEventDataBuffer event{XR_TYPE_EVENT_DATA_BUFFER};
            auto status=fn<PFN_xrPollEvent>("xrPollEvent")(instance,&event);
            if(status==XR_EVENT_UNAVAILABLE) return;
            xr(status,"xrPollEvent");
            if(event.type==XR_TYPE_EVENT_DATA_INSTANCE_LOSS_PENDING) { exit=true; return; }
            if(event.type!=XR_TYPE_EVENT_DATA_SESSION_STATE_CHANGED) continue;
            auto& change=*reinterpret_cast<XrEventDataSessionStateChanged*>(&event);
            if(change.session!=session) continue;
            state=change.state; focused=state==XR_SESSION_STATE_FOCUSED;
            std::cout<<"Session state: "<<state<<'\n';
            if(state==XR_SESSION_STATE_READY&&!running) {
                XrSessionBeginInfo begin{XR_TYPE_SESSION_BEGIN_INFO}; begin.primaryViewConfigurationType=XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;
                xr(fn<PFN_xrBeginSession>("xrBeginSession")(session,&begin),"xrBeginSession"); running=true;
            } else if(state==XR_SESSION_STATE_STOPPING&&running) {
                xr(fn<PFN_xrEndSession>("xrEndSession")(session),"xrEndSession"); running=false;
            } else if(state==XR_SESSION_STATE_EXITING||state==XR_SESSION_STATE_LOSS_PENDING) exit=true;
        }
    }
};


}

