#include "openxr_d3d11.hpp"
using namespace copvr;
struct Vertex { Clip position; Vec3 color; };
struct Scene {
    Host& host; Com<ID3D11VertexShader> vs; Com<ID3D11PixelShader> ps;
    Com<ID3D11InputLayout> layout; Com<ID3D11Buffer> buffer; Com<ID3D11RasterizerState> raster;
    Com<ID3D11DepthStencilState> depth;
    std::vector<Item> items; std::vector<Slot> slots; Interaction interaction;
    bool placed{};
    Scene(Host& h):host(h) {
        const char* source=R"(
struct V { float4 p:SV_POSITION; float3 c:COLOR; };
V VS(float4 p:POSITION,float3 c:COLOR) { V o; o.p=p; o.c=c; return o; }
float4 PS(V v):SV_TARGET { return float4(v.c,1); }
)";
        Com<ID3DBlob> vcode,pcode,errors;
        auto compile=[&](const char* entry,const char* target,Com<ID3DBlob>& code) {
            HRESULT status=D3DCompile(source,std::strlen(source),nullptr,nullptr,nullptr,entry,target,0,0,code.put(),errors.put());
            if(FAILED(status)&&errors.p) std::cerr<<static_cast<const char*>(errors->GetBufferPointer()); hr(status,"compile diagnostic shader");
        };
        compile("VS","vs_5_0",vcode); compile("PS","ps_5_0",pcode);
        hr(h.device->CreateVertexShader(vcode->GetBufferPointer(),vcode->GetBufferSize(),nullptr,vs.put()),"vertex shader");
        hr(h.device->CreatePixelShader(pcode->GetBufferPointer(),pcode->GetBufferSize(),nullptr,ps.put()),"pixel shader");
        D3D11_INPUT_ELEMENT_DESC attributes[]={{"POSITION",0,DXGI_FORMAT_R32G32B32A32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0},
            {"COLOR",0,DXGI_FORMAT_R32G32B32_FLOAT,0,16,D3D11_INPUT_PER_VERTEX_DATA,0}};
        hr(h.device->CreateInputLayout(attributes,2,vcode->GetBufferPointer(),vcode->GetBufferSize(),layout.put()),"input layout");
        D3D11_BUFFER_DESC desc{}; desc.ByteWidth=sizeof(Vertex)*65536; desc.Usage=D3D11_USAGE_DYNAMIC;
        desc.BindFlags=D3D11_BIND_VERTEX_BUFFER; desc.CPUAccessFlags=D3D11_CPU_ACCESS_WRITE;
        hr(h.device->CreateBuffer(&desc,nullptr,buffer.put()),"vertex buffer");
        D3D11_RASTERIZER_DESC rd{}; rd.FillMode=D3D11_FILL_SOLID; rd.CullMode=D3D11_CULL_NONE; rd.DepthClipEnable=TRUE;
        hr(h.device->CreateRasterizerState(&rd,raster.put()),"rasterizer");
        D3D11_DEPTH_STENCIL_DESC ds{}; ds.DepthEnable=FALSE;
        hr(h.device->CreateDepthStencilState(&ds,depth.put()),"depth state");
    }
    void update(const Frame& frame) {
        if(!placed&&frame.focused&&frame.head.valid) {
            auto p=frame.head.position;
            items.push_back({1,{{p.x,p.y-.35f,p.z-.45f},{},true},Location::World,-1,true});
            slots.push_back({{p.x-.25f,p.y-.45f,p.z-.35f},.16f,{}}); placed=true;
        }
        for(const auto& event:interaction.update(frame,items,slots))
            std::cout<<"Interaction "<<static_cast<int>(event.type)<<" item="<<event.item<<" hand="<<event.hand<<'\n';
    }
    void draw(ID3D11Texture2D* target,const Eye& eye,const XrView& view,const Frame& frame) {
        Com<ID3D11RenderTargetView> rtv;
        D3D11_RENDER_TARGET_VIEW_DESC targetView{};
        targetView.Format=eye.format; targetView.ViewDimension=D3D11_RTV_DIMENSION_TEXTURE2D;
        hr(host.device->CreateRenderTargetView(target,&targetView,rtv.put()),"eye RTV");
        auto* ctx=host.context.p; ctx->OMSetRenderTargets(1,&rtv.p,nullptr);
        const float background[]={.025f,.03f,.04f,1}; ctx->ClearRenderTargetView(rtv.p,background);
        D3D11_VIEWPORT viewport{0,0,static_cast<float>(eye.width),static_cast<float>(eye.height),0,1}; ctx->RSSetViewports(1,&viewport);
        Pose camera{{view.pose.position.x,view.pose.position.y,view.pose.position.z},
            {view.pose.orientation.x,view.pose.orientation.y,view.pose.orientation.z,view.pose.orientation.w},true};
        Frustum f{view.fov.angleLeft,view.fov.angleRight,view.fov.angleUp,view.fov.angleDown};
        std::vector<Vertex> vertices;
        auto cube=[&](Pose pose,Vec3 scale,Vec3 color) {
            static const int triangles[]={0,1,2,2,1,3,4,6,5,5,6,7,0,4,1,1,4,5,2,3,6,6,3,7,0,2,4,4,2,6,1,5,3,3,5,7};
            for(int index:triangles) {
                Vec3 v{(index&1?1.f:-1.f)*scale.x,(index&2?1.f:-1.f)*scale.y,(index&4?1.f:-1.f)*scale.z};
                vertices.push_back({project(pose.position+rotate(pose.orientation,v),camera,f),color});
            }
        };
        // Static markers, not a replacement for the game renderer.
        for(int x=-2;x<=2;++x) for(int z=-3;z<=0;++z)
            cube({{static_cast<float>(x),-1.5f,static_cast<float>(z)},{},true},{.025f,.025f,.025f},{.2f,.25f,.2f});
        for(const auto& slot:slots) cube({slot.position,{},true},{.08f,.015f,.08f},{.25f,.6f,.25f});
        for(const auto& item:items) cube(item.pose,{.035f,.035f,.08f},{.7f,.5f,.15f});
        for(int h=0;h<2;++h) {
            const auto& hand=frame.hands[h];
            if(hand.grip.valid) cube(hand.grip,{.025f,.025f,.06f},h==0?Vec3{.15f,.4f,.8f}:Vec3{.8f,.25f,.15f});
            if(hand.aim.valid) {
                auto aim=hand.aim; aim.position=aim.position+rotate(aim.orientation,{0,0,-.18f});
                cube(aim,{.003f,.003f,.12f},{.9f,.9f,.4f});
            }
        }
        if(vertices.size()>65536) throw std::runtime_error("Diagnostic vertex limit");
        D3D11_MAPPED_SUBRESOURCE map{}; hr(ctx->Map(buffer.p,0,D3D11_MAP_WRITE_DISCARD,0,&map),"map vertices");
        std::memcpy(map.pData,vertices.data(),vertices.size()*sizeof(Vertex)); ctx->Unmap(buffer.p,0);
        UINT stride=sizeof(Vertex),offset=0;
        ctx->IASetVertexBuffers(0,1,&buffer.p,&stride,&offset); ctx->IASetInputLayout(layout.p);
        ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        ctx->VSSetShader(vs.p,nullptr,0); ctx->PSSetShader(ps.p,nullptr,0);
        ctx->RSSetState(raster.p); ctx->OMSetDepthStencilState(depth.p,0); ctx->Draw(static_cast<UINT>(vertices.size()),0);
        ctx->OMSetRenderTargets(0,nullptr,nullptr);
    }
};

int run(Host& host,int seconds) {
    Scene scene(host); auto start=std::chrono::steady_clock::now();
    bool requestedExit=false; uint64_t rendered{},headFrames{},leftFrames{},rightFrames{};
    while(!host.exit) {
        host.events(); if(host.exit) break;
        const bool timeout=std::chrono::steady_clock::now()-start>std::chrono::seconds(seconds);
        if((timeout||(GetAsyncKeyState(VK_ESCAPE)&0x8000))&&!requestedExit) {
            if(!host.running) break;
            xr(host.fn<PFN_xrRequestExitSession>("xrRequestExitSession")(host.session),"request session exit"); requestedExit=true;
        }
        if(requestedExit&&std::chrono::steady_clock::now()-start>std::chrono::seconds(seconds+5)) break;
        if(!host.running) { Sleep(10); continue; }
        XrFrameWaitInfo wait{XR_TYPE_FRAME_WAIT_INFO}; XrFrameState frameState{XR_TYPE_FRAME_STATE};
        xr(host.fn<PFN_xrWaitFrame>("xrWaitFrame")(host.session,&wait,&frameState),"xrWaitFrame");
        XrFrameBeginInfo begin{XR_TYPE_FRAME_BEGIN_INFO};
        xr(host.fn<PFN_xrBeginFrame>("xrBeginFrame")(host.session,&begin),"xrBeginFrame");
        auto frame=host.input->sample(frameState.predictedDisplayTime,host.focused); scene.update(frame);
        headFrames+=frame.head.valid; leftFrames+=frame.hands[0].grip.valid; rightFrames+=frame.hands[1].grip.valid;
        XrViewLocateInfo locate{XR_TYPE_VIEW_LOCATE_INFO}; locate.viewConfigurationType=XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;
        locate.displayTime=frameState.predictedDisplayTime; locate.space=host.space;
        XrViewState viewState{XR_TYPE_VIEW_STATE}; uint32_t count{};
        std::array<XrView,2> views{{{XR_TYPE_VIEW},{XR_TYPE_VIEW}}};
        xr(host.fn<PFN_xrLocateViews>("xrLocateViews")(host.session,&locate,&viewState,2,&count,views.data()),"xrLocateViews");
        constexpr auto valid=XR_VIEW_STATE_POSITION_VALID_BIT|XR_VIEW_STATE_ORIENTATION_VALID_BIT;
        std::array<XrCompositionLayerProjectionView,2> projectionViews{{{XR_TYPE_COMPOSITION_LAYER_PROJECTION_VIEW},{XR_TYPE_COMPOSITION_LAYER_PROJECTION_VIEW}}};
        bool submit=frameState.shouldRender&&count==2&&(viewState.viewStateFlags&valid)==valid;
        if(submit) for(int i=0;i<2;++i) {
            auto& eye=host.eyes[i]; uint32_t image{};
            XrSwapchainImageAcquireInfo acquire{XR_TYPE_SWAPCHAIN_IMAGE_ACQUIRE_INFO};
            xr(host.fn<PFN_xrAcquireSwapchainImage>("xrAcquireSwapchainImage")(eye.swapchain,&acquire,&image),"acquire eye image");
            XrSwapchainImageWaitInfo waitImage{XR_TYPE_SWAPCHAIN_IMAGE_WAIT_INFO}; waitImage.timeout=XR_INFINITE_DURATION;
            xr(host.fn<PFN_xrWaitSwapchainImage>("xrWaitSwapchainImage")(eye.swapchain,&waitImage),"wait eye image");
            scene.draw(eye.images.at(image).texture,eye,views[i],frame);
            XrSwapchainImageReleaseInfo release{XR_TYPE_SWAPCHAIN_IMAGE_RELEASE_INFO};
            xr(host.fn<PFN_xrReleaseSwapchainImage>("xrReleaseSwapchainImage")(eye.swapchain,&release),"release eye image");
            auto& p=projectionViews[i]; p.pose=views[i].pose; p.fov=views[i].fov;
            p.subImage.swapchain=eye.swapchain; p.subImage.imageRect.extent={static_cast<int32_t>(eye.width),static_cast<int32_t>(eye.height)};
        }
        XrCompositionLayerProjection layer{XR_TYPE_COMPOSITION_LAYER_PROJECTION}; layer.space=host.space;
        layer.viewCount=2; layer.views=projectionViews.data();
        const auto* base=reinterpret_cast<XrCompositionLayerBaseHeader*>(&layer);
        XrFrameEndInfo end{XR_TYPE_FRAME_END_INFO}; end.displayTime=frameState.predictedDisplayTime; end.environmentBlendMode=XR_ENVIRONMENT_BLEND_MODE_OPAQUE;
        end.layerCount=submit?1:0; end.layers=submit?&base:nullptr;
        xr(host.fn<PFN_xrEndFrame>("xrEndFrame")(host.session,&end),"xrEndFrame");
        if(submit) ++rendered;
    }
    std::cout<<"Submitted stereo frames: "<<rendered<<'\n';
    std::cout<<"Tracked frames: head="<<headFrames<<" left="<<leftFrames<<" right="<<rightFrames<<'\n';
    return rendered?0:4;
}
int main(int argc,char** argv) {
    std::cout<<std::unitbuf;
    try {
        bool render=false,sharedProbe=false; int seconds=60;
        if(argc>=2) {
            if(std::strcmp(argv[1],"--run")==0) { render=true; if(argc==3) seconds=std::stoi(argv[2]); }
            else if(std::strcmp(argv[1],"--probe-shared-device")==0) sharedProbe=true;
            else if(std::strcmp(argv[1],"--probe")!=0) throw std::runtime_error("Usage: vr_diagnostic.exe --probe | --probe-shared-device | --run [seconds]");
        }
        if(seconds<1||seconds>600||argc>3||(!render&&argc>2)) throw std::runtime_error("Invalid arguments; duration must be 1..600 seconds");
        Host host;
        if(!host.initialize(render)) return 3;
        if(sharedProbe) {
            auto requirements=host.graphicsRequirements();
            Com<IDXGIFactory4> factory; Com<IDXGIAdapter1> adapter;
            hr(CreateDXGIFactory1(__uuidof(IDXGIFactory4),reinterpret_cast<void**>(factory.put())),"external DXGI factory");
            hr(factory->EnumAdapterByLuid(requirements.adapterLuid,__uuidof(IDXGIAdapter1),reinterpret_cast<void**>(adapter.put())),"external adapter");
            Com<ID3D11Device> externalDevice; Com<ID3D11DeviceContext> externalContext;
            D3D_FEATURE_LEVEL level=D3D_FEATURE_LEVEL_11_0;
            hr(D3D11CreateDevice(adapter.p,D3D_DRIVER_TYPE_UNKNOWN,nullptr,D3D11_CREATE_DEVICE_BGRA_SUPPORT,
                &level,1,D3D11_SDK_VERSION,externalDevice.put(),nullptr,externalContext.put()),"external device");
            host.initializeGraphics(externalDevice.p);
            if(host.device.p!=externalDevice.p) throw std::runtime_error("Host failed to retain the external device");
            std::cout<<"SHARED_DEVICE_OK: session and eye swapchains use caller-owned D3D11 device; rendering not tested.\n";
            return 0;
        }
        if(!render) { std::cout<<"PROBE_OK: headset system found; rendering and controller tracking not tested.\n"; return 0; }
        return run(host,seconds);
    } catch(const std::exception& e) { std::cerr<<"ERROR: "<<e.what()<<'\n'; return 1; }
}

