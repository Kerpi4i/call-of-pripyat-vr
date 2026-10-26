#include "vr_shadow_atlas_cache.hpp"
#undef NDEBUG
#include <cassert>
#include <cmath>
float depthAt(const D3D11_MAPPED_SUBRESOURCE& map,DXGI_FORMAT format,unsigned x,unsigned y){
    const auto* row=static_cast<const unsigned char*>(map.pData)+y*map.RowPitch;
    if(format==DXGI_FORMAT_R24G8_TYPELESS)return float(reinterpret_cast<const uint32_t*>(row)[x]&0xffffff)/16777215.f;
    if(format==DXGI_FORMAT_R16_TYPELESS)return float(reinterpret_cast<const uint16_t*>(row)[x])/65535.f;
    return reinterpret_cast<const float*>(row)[x];
}
void run(DXGI_FORMAT format){
    ID3D11Device* device{}; ID3D11DeviceContext* ctx{};
    const auto hr=D3D11CreateDevice(nullptr,D3D_DRIVER_TYPE_WARP,nullptr,0,nullptr,0,D3D11_SDK_VERSION,&device,nullptr,&ctx);
    assert(SUCCEEDED(hr));
    D3D11_TEXTURE2D_DESC desc{};desc.Width=64;desc.Height=64;desc.MipLevels=1;desc.ArraySize=2;
    desc.Format=format;desc.SampleDesc.Count=1;desc.Usage=D3D11_USAGE_DEFAULT;
    desc.BindFlags=D3D11_BIND_DEPTH_STENCIL|D3D11_BIND_SHADER_RESOURCE;
    ID3D11Texture2D* source{};assert(SUCCEEDED(device->CreateTexture2D(&desc,nullptr,&source)));
    D3D11_DEPTH_STENCIL_VIEW_DESC vd{};vd.Format=format==DXGI_FORMAT_R32_TYPELESS?DXGI_FORMAT_D32_FLOAT:(format==DXGI_FORMAT_R24G8_TYPELESS?DXGI_FORMAT_D24_UNORM_S8_UINT:DXGI_FORMAT_D16_UNORM);vd.ViewDimension=D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
    vd.Texture2DArray.ArraySize=1;
    ID3D11DepthStencilView* view{};assert(SUCCEEDED(device->CreateDepthStencilView(source,&vd,&view)));
    {
        copvr::ShadowAtlasCache cache;
        assert(cache.prepare(1,source));
        ctx->ClearDepthStencilView(view,D3D11_CLEAR_DEPTH,.25f,0);
        assert(cache.store(device,ctx,"exact-key",{12,34}));
        ctx->ClearDepthStencilView(view,D3D11_CLEAR_DEPTH,.75f,0);
        std::vector<uintptr_t> lights;
        assert(!cache.restore(ctx,"other-key",lights));
        assert(cache.restore(ctx,"exact-key",lights));
        assert((lights==std::vector<uintptr_t>{12,34}));
        auto read=desc;read.Usage=D3D11_USAGE_STAGING;read.BindFlags=0;read.CPUAccessFlags=D3D11_CPU_ACCESS_READ;
        ID3D11Texture2D* staging{};assert(SUCCEEDED(device->CreateTexture2D(&read,nullptr,&staging)));
        ctx->CopyResource(staging,source);
        D3D11_MAPPED_SUBRESOURCE mapped{};assert(SUCCEEDED(ctx->Map(staging,0,D3D11_MAP_READ,0,&mapped)));
        assert(std::abs(depthAt(mapped,format,0,0)-.25f)<.0001f);
        ctx->Unmap(staging,0);staging->Release();
        assert(cache.prepare(2,source));assert(!cache.restore(ctx,"exact-key",lights));
        for(int i=0;i<8;++i)assert(cache.store(device,ctx,std::to_string(i),{}));
        assert(!cache.store(device,ctx,"over-capacity",{}));
        assert(cache.prepare(3,source));
        const copvr::ShadowCacheKeyParts parts{"context","members","sizes","transforms","layout"};
        assert(std::string(cache.missReason(parts))=="shadow_cache_miss_cold");
        assert(cache.store(device,ctx,"diagnostic",{},parts));
        for(int field=0;field<5;++field) {
            auto changed=parts;
            std::string* fields[]={&changed.context,&changed.members,&changed.sizes,&changed.transforms,&changed.layout};
            *fields[field]+="different";
            const char* reasons[]={"context","members","sizes","transforms","layout"};
            assert(std::string(cache.missReason(changed))==std::string("shadow_cache_miss_")+reasons[field]);
        }
        assert(cache.prepare(4,source));
        const std::vector<copvr::ShadowCacheTile> tiles{{12,"rect-A",0,0,8},{34,"rect-B",48,48,8}};
        assert(cache.store(device,ctx,"larger-batch",{12,34},parts,tiles));
        assert(cache.restoreSubset(ctx,"context",{tiles[0]},lights));
        assert((lights==std::vector<uintptr_t>{12}));
        assert(cache.restoreSubset(ctx,"context",{tiles[1],tiles[0]},lights));
        assert(!cache.restoreSubset(ctx,"other-context",{tiles[0]},lights));
        auto moved=tiles[0];moved.key="changed-rect";
        assert(!cache.restoreSubset(ctx,"context",{moved},lights));
        assert(!cache.restoreSubset(ctx,"context",{{56,"new-light",24,24,8}},lights));
        assert(cache.prepare(5,source));
        const std::vector<copvr::ShadowCacheTile> adjacent{{12,"A",0,0,8},{34,"B",8,0,8}};
        assert(cache.store(device,ctx,"adjacent",{12,34},parts,adjacent));
        assert(!cache.restoreSubset(ctx,"context",{adjacent[0]},lights));
        assert(cache.prepare(6,source));
        ctx->ClearState();ctx->ClearDepthStencilView(view,D3D11_CLEAR_DEPTH,.25f,0);
        assert(cache.store(device,ctx,"tile-source-A",{12},parts,{{12,"layout-A",0,0,8,"content-A"}}));
        ctx->ClearDepthStencilView(view,D3D11_CLEAR_DEPTH,.6f,0);
        assert(cache.store(device,ctx,"tile-source-B",{34},parts,{{34,"layout-B",0,0,8,"content-B"}}));
        ctx->ClearDepthStencilView(view,D3D11_CLEAR_DEPTH,1.f,0);
        std::vector<uintptr_t> restored;
        const std::vector<copvr::ShadowCacheTile> requested{
            {12,"new-layout-A",16,16,8,"content-A"},
            {34,"new-layout-B",40,40,8,"content-B"},
            {56,"missing",0,48,8,"missing"}};
        assert(cache.restoreTiles(device,ctx,"context",requested,restored,lights));
        assert((restored==std::vector<uintptr_t>{12,34}));
        assert((lights==std::vector<uintptr_t>{12,34}));
        ctx->ClearState();
        assert(SUCCEEDED(device->CreateTexture2D(&read,nullptr,&staging)));
        ctx->CopyResource(staging,source);
        assert(SUCCEEDED(ctx->Map(staging,0,D3D11_MAP_READ,0,&mapped)));
        auto value=[&](unsigned x,unsigned y){return depthAt(mapped,format,x,y);};
        assert(std::abs(value(16,16)-.25f)<.0001f);
        assert(std::abs(value(23,23)-.25f)<.0001f);
        assert(std::abs(value(40,40)-.6f)<.0001f);
        assert(value(0,0)==1.f&&value(24,24)==1.f&&value(48,48)==1.f);
        ctx->Unmap(staging,0);staging->Release();
        auto changed=requested[0];changed.contentKey="changed-matrix";
        assert(!cache.restoreTiles(device,ctx,"context",{changed},restored,lights));
    }
    view->Release();source->Release();ctx->Release();device->Release();
}

int main(){for(auto format:{DXGI_FORMAT_R32_TYPELESS,DXGI_FORMAT_R24G8_TYPELESS,DXGI_FORMAT_R16_TYPELESS})run(format);}
