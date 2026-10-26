#pragma once
#include <d3d11.h>
#include <d3dcompiler.h>
#include <cstring>
namespace copvr {
// Raw immediate-context helper. Caller unbinds resources and invalidates its
// state cache before/after use. Depth stays on GPU; no readback or Flush.
class DepthTileBlit {
    ID3D11VertexShader* vs_{};ID3D11PixelShader* ps_{};
    ID3D11Buffer* constants_{};ID3D11DepthStencilState* depth_{};
    ID3D11RasterizerState* raster_{};ID3D11DepthStencilView* target_{};
    bool attempted_{},ready_{};
public:
    ~DepthTileBlit(){if(vs_)vs_->Release();if(ps_)ps_->Release();if(constants_)constants_->Release();
        if(depth_)depth_->Release();if(raster_)raster_->Release();if(target_)target_->Release();}
    bool initialize(ID3D11Device* device,ID3D11Texture2D* target) {
        if(attempted_)return ready_;
        attempted_=true;
        const auto module=LoadLibraryA("d3dcompiler_47.dll");if(!module)return false;
        const auto compile=reinterpret_cast<decltype(&D3DCompile)>(GetProcAddress(module,"D3DCompile"));
        if(!compile){FreeLibrary(module);return false;}
        const char* source=R"(
Texture2D<float> savedDepth:register(t0);
cbuffer CopyOffset:register(b0){int2 offset;int2 padding;};
float4 vertex(uint id:SV_VertexID):SV_Position {
    float2 uv=float2((id<<1)&2,id&2);
    return float4(uv*float2(2,-2)+float2(-1,1),0,1);
}
float pixel(float4 pos:SV_Position):SV_Depth {
    return savedDepth.Load(int3(int2(pos.xy)+offset,0));
})";
        ID3DBlob *vertex{},*pixel{},*error{};
        auto a=compile(source,std::strlen(source),"copvr-depth-tile",nullptr,nullptr,"vertex","vs_4_0",D3DCOMPILE_OPTIMIZATION_LEVEL3,0,&vertex,&error);
        if(error){OutputDebugStringA(static_cast<const char*>(error->GetBufferPointer()));error->Release();error=nullptr;}
        auto b=compile(source,std::strlen(source),"copvr-depth-tile",nullptr,nullptr,"pixel","ps_4_0",D3DCOMPILE_OPTIMIZATION_LEVEL3,0,&pixel,&error);
        if(error){OutputDebugStringA(static_cast<const char*>(error->GetBufferPointer()));error->Release();}
        bool ok=SUCCEEDED(a)&&SUCCEEDED(b);
        if(ok)ok=SUCCEEDED(device->CreateVertexShader(vertex->GetBufferPointer(),vertex->GetBufferSize(),nullptr,&vs_))&&
                  SUCCEEDED(device->CreatePixelShader(pixel->GetBufferPointer(),pixel->GetBufferSize(),nullptr,&ps_));
        if(vertex)vertex->Release();if(pixel)pixel->Release();FreeLibrary(module);if(!ok)return false;
        D3D11_BUFFER_DESC buffer{};buffer.ByteWidth=16;buffer.Usage=D3D11_USAGE_DEFAULT;buffer.BindFlags=D3D11_BIND_CONSTANT_BUFFER;
        if(FAILED(device->CreateBuffer(&buffer,nullptr,&constants_)))return false;
        D3D11_DEPTH_STENCIL_DESC depth{};depth.DepthEnable=TRUE;depth.DepthWriteMask=D3D11_DEPTH_WRITE_MASK_ALL;depth.DepthFunc=D3D11_COMPARISON_ALWAYS;
        if(FAILED(device->CreateDepthStencilState(&depth,&depth_)))return false;
        D3D11_RASTERIZER_DESC raster{};raster.FillMode=D3D11_FILL_SOLID;raster.CullMode=D3D11_CULL_NONE;raster.DepthClipEnable=TRUE;
        if(FAILED(device->CreateRasterizerState(&raster,&raster_)))return false;
        D3D11_TEXTURE2D_DESC desc{};target->GetDesc(&desc);
        D3D11_DEPTH_STENCIL_VIEW_DESC view{};
        switch(desc.Format){
        case DXGI_FORMAT_R32_TYPELESS:view.Format=DXGI_FORMAT_D32_FLOAT;break;
        case DXGI_FORMAT_R24G8_TYPELESS:view.Format=DXGI_FORMAT_D24_UNORM_S8_UINT;break;
        case DXGI_FORMAT_R16_TYPELESS:view.Format=DXGI_FORMAT_D16_UNORM;break;
        default:return false;
        }
        if(desc.ArraySize>1){view.ViewDimension=D3D11_DSV_DIMENSION_TEXTURE2DARRAY;view.Texture2DArray.ArraySize=1;}
        else view.ViewDimension=D3D11_DSV_DIMENSION_TEXTURE2D;
        ready_=SUCCEEDED(device->CreateDepthStencilView(target,&view,&target_));return ready_;
    }
    void copy(ID3D11DeviceContext* ctx,ID3D11ShaderResourceView* source,unsigned fromX,unsigned fromY,
              unsigned toX,unsigned toY,unsigned size) {
        const int offset[4]={int(fromX)-int(toX),int(fromY)-int(toY),0,0};
        ctx->UpdateSubresource(constants_,0,nullptr,offset,0,0);
        ctx->OMSetRenderTargets(0,nullptr,target_);ctx->OMSetDepthStencilState(depth_,0);
        ctx->RSSetState(raster_);const D3D11_VIEWPORT viewport{float(toX),float(toY),float(size),float(size),0,1};ctx->RSSetViewports(1,&viewport);
        ctx->IASetInputLayout(nullptr);ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        ctx->VSSetShader(vs_,nullptr,0);ctx->PSSetShader(ps_,nullptr,0);
        ctx->PSSetConstantBuffers(0,1,&constants_);ctx->PSSetShaderResources(0,1,&source);ctx->Draw(3,0);
        ID3D11ShaderResourceView* empty{};ctx->PSSetShaderResources(0,1,&empty);
    }
};
}
