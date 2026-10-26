#pragma once
#include <d3d11.h>
#include <string>
#include <vector>
#include <memory>
#include <cstdint>
#include <algorithm>
#include "vr_depth_tile_blit.hpp"
namespace copvr {
struct ShadowCacheTile {uintptr_t light{};std::string key;uint32_t x{},y{},size{};std::string contentKey;};
struct ShadowCacheKeyParts {
    std::string context,members,sizes,transforms,layout;
};
// One simulation frame only. Exact keys, single depth slice, bounded VRAM.
class ShadowAtlasCache {
    struct Entry {
        ID3D11Texture2D* texture{};
        ID3D11ShaderResourceView* view{};
        std::string key;
        ShadowCacheKeyParts parts;
        std::vector<ShadowCacheTile> tiles;
        std::vector<uintptr_t> contributors;
        bool valid{};
        ~Entry(){if(view)view->Release();if(texture)texture->Release();}
    };
    std::vector<std::unique_ptr<Entry>> entries_;
    ID3D11Texture2D* source_{};
    D3D11_TEXTURE2D_DESC desc_{};
    uint64_t frame_=~uint64_t(0);
    size_t capacity_{};
    std::unique_ptr<DepthTileBlit> blit_;
public:
    ~ShadowAtlasCache(){if(source_)source_->Release();}
    bool prepare(uint64_t frame,ID3D11Texture2D* source) {
        if(!source)return false;
        if(source!=source_) {
            blit_.reset();
            entries_.clear();if(source_)source_->Release();
            source_=source;source_->AddRef();source_->GetDesc(&desc_);
            frame_=~uint64_t(0);capacity_=0;
            unsigned bytes=0;
            switch(desc_.Format) {
            case DXGI_FORMAT_R24G8_TYPELESS:case DXGI_FORMAT_R32_TYPELESS:bytes=4;break;
            case DXGI_FORMAT_R16_TYPELESS:bytes=2;break;
            default:return false;
            }
            if(desc_.SampleDesc.Count!=1||desc_.MipLevels!=1)return false;
            const uint64_t perAtlas=uint64_t(desc_.Width)*desc_.Height*bytes;
            capacity_=perAtlas?size_t((128ull*1024*1024)/perAtlas):0;
            if(capacity_>8)capacity_=8;
        }
        if(frame_!=frame) {for(auto& e:entries_)e->valid=false;frame_=frame;}
        return capacity_!=0;
    }
    bool restore(ID3D11DeviceContext* ctx,const std::string& key,std::vector<uintptr_t>& contributors) {
        for(auto& e:entries_)if(e->valid&&e->key==key) {
            ctx->CopySubresourceRegion(source_,0,0,0,0,e->texture,0,nullptr);
            contributors=e->contributors;return true;
        }
        return false;
    }
    const char* missReason(const ShadowCacheKeyParts& parts) const {
        const Entry* closest=nullptr;unsigned best=0;
        for(const auto& e:entries_)if(e->valid) {
            const unsigned score=(e->parts.members==parts.members?16:0)+
                (e->parts.context==parts.context?8:0)+(e->parts.sizes==parts.sizes?4:0)+
                (e->parts.transforms==parts.transforms?2:0)+(e->parts.layout==parts.layout?1:0);
            if(!closest||score>best){closest=e.get();best=score;}
        }
        if(!closest)return "shadow_cache_miss_cold";
        const auto& other=closest->parts;
        if(other.members!=parts.members)return "shadow_cache_miss_members";
        if(other.context!=parts.context)return "shadow_cache_miss_context";
        if(other.sizes!=parts.sizes)return "shadow_cache_miss_sizes";
        if(other.transforms!=parts.transforms)return "shadow_cache_miss_transforms";
        if(other.layout!=parts.layout)return "shadow_cache_miss_layout";
        return "shadow_cache_miss_other";
    }
    bool restoreSubset(ID3D11DeviceContext* ctx,const std::string& context,
                       const std::vector<ShadowCacheTile>& tiles,std::vector<uintptr_t>& contributors) {
        if(tiles.empty())return false;
        for(const auto& entry:entries_)if(entry->valid&&entry->parts.context==context) {
            bool complete=true;
            for(const auto& tile:tiles) {
                const auto match=std::find_if(entry->tiles.begin(),entry->tiles.end(),[&](const ShadowCacheTile& saved){
                    return saved.light==tile.light&&saved.key==tile.key;
                });
                if(match==entry->tiles.end()){complete=false;break;}
            }
            if(!complete)continue;
            // Reject extra populated tiles near requested tile borders: PCF
            // taps can extend beyond the nominal atlas rectangle.
            for(const auto& extra:entry->tiles) {
                const bool requested=std::any_of(tiles.begin(),tiles.end(),[&](const ShadowCacheTile& tile){return tile.light==extra.light;});
                if(requested||std::find(entry->contributors.begin(),entry->contributors.end(),extra.light)==entry->contributors.end())continue;
                for(const auto& tile:tiles) {
                    if(!tile.size||!extra.size){complete=false;break;}
                    const int64_t margin=16;
                    if(int64_t(tile.x)-margin<int64_t(extra.x)+extra.size&&int64_t(extra.x)<int64_t(tile.x)+tile.size+margin&&
                       int64_t(tile.y)-margin<int64_t(extra.y)+extra.size&&int64_t(extra.y)<int64_t(tile.y)+tile.size+margin){complete=false;break;}
                }
                if(!complete)break;
            }
            if(!complete)continue;
            // Unused atlas rectangles do not affect lighting. Every rectangle
            // actually sampled by this batch has an exact matching saved tile.
            ctx->CopySubresourceRegion(source_,0,0,0,0,entry->texture,0,nullptr);
            contributors.clear();
            for(const auto& tile:tiles)
                if(std::find(entry->contributors.begin(),entry->contributors.end(),tile.light)!=entry->contributors.end())
                    contributors.push_back(tile.light);
            return true;
        }
        return false;
    }
    bool restoreTiles(ID3D11Device* device,ID3D11DeviceContext* ctx,const std::string& context,
                      const std::vector<ShadowCacheTile>& tiles,std::vector<uintptr_t>& restored,
                      std::vector<uintptr_t>& contributors) {
        struct Match {Entry* entry;const ShadowCacheTile* saved;const ShadowCacheTile* requested;};
        std::vector<Match> matches;
        for(const auto& tile:tiles) {
            if(!tile.size||tile.x>desc_.Width||tile.size>desc_.Width-tile.x||
               tile.y>desc_.Height||tile.size>desc_.Height-tile.y)continue;
            bool found=false;
            for(const auto& entry:entries_)if(entry->valid&&entry->view&&entry->parts.context==context) {
                for(const auto& saved:entry->tiles)
                    if(saved.light==tile.light&&saved.size==tile.size&&
                       (saved.contentKey.empty()?saved.key:saved.contentKey)==(tile.contentKey.empty()?tile.key:tile.contentKey)) {
                        matches.push_back({entry.get(),&saved,&tile});found=true;break;
                    }
                if(found)break;
            }
        }
        if(matches.empty())return false;
        if(!blit_)blit_=std::make_unique<DepthTileBlit>();
        if(!blit_->initialize(device,source_))return false;
        restored.clear();contributors.clear();
        for(const auto& match:matches) {
            const auto& from=*match.saved;const auto& to=*match.requested;
            blit_->copy(ctx,match.entry->view,from.x,from.y,to.x,to.y,to.size);
            restored.push_back(to.light);
            if(std::find(match.entry->contributors.begin(),match.entry->contributors.end(),to.light)!=match.entry->contributors.end())
                contributors.push_back(to.light);
        }
        return true;
    }
    bool store(ID3D11Device* device,ID3D11DeviceContext* ctx,const std::string& key,const std::vector<uintptr_t>& contributors,
               const ShadowCacheKeyParts& parts={},const std::vector<ShadowCacheTile>& tiles={}) {
        Entry* entry=nullptr;
        for(auto& e:entries_)if(!e->valid){entry=e.get();break;}
        if(!entry) {
            if(entries_.size()>=capacity_)return false;
            auto e=std::make_unique<Entry>();
            auto d=desc_;d.ArraySize=1;d.BindFlags=D3D11_BIND_SHADER_RESOURCE;d.MiscFlags=0;
            d.Usage=D3D11_USAGE_DEFAULT;d.CPUAccessFlags=0;
            if(FAILED(device->CreateTexture2D(&d,nullptr,&e->texture)))return false;
            D3D11_SHADER_RESOURCE_VIEW_DESC view{};view.ViewDimension=D3D11_SRV_DIMENSION_TEXTURE2D;view.Texture2D.MipLevels=1;
            switch(d.Format){
            case DXGI_FORMAT_R32_TYPELESS:view.Format=DXGI_FORMAT_R32_FLOAT;break;
            case DXGI_FORMAT_R24G8_TYPELESS:view.Format=DXGI_FORMAT_R24_UNORM_X8_TYPELESS;break;
            case DXGI_FORMAT_R16_TYPELESS:view.Format=DXGI_FORMAT_R16_UNORM;break;
            default:return false;
            }
            if(FAILED(device->CreateShaderResourceView(e->texture,&view,&e->view)))return false;
            entry=e.get();entries_.push_back(std::move(e));
        }
        ctx->CopySubresourceRegion(entry->texture,0,0,0,0,source_,0,nullptr);
        entry->key=key;entry->parts=parts;entry->tiles=tiles;entry->contributors=contributors;entry->valid=true;return true;
    }
};
}
