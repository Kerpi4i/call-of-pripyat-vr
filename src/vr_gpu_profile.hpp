#pragma once
#include "vr_profile.hpp"
#include <d3d11.h>
#include <array>
#include <string>
namespace copvr {
// Immediate-context owner only. Never flush or wait for a query result.
class GpuProfile {
    struct Slot {
        struct Marker { ID3D11Query* query{}; std::string name; bool opening{}; UINT64 value{}; double cpuStart{}; };
        std::array<Marker,1024> markers{};
        unsigned markerCount{};
        bool markerOverflow{};
        ID3D11Query *disjoint{},*start{},*end{};
        bool pending{}; uint64_t frame{}; double cpuStart{};
        ~Slot(){for(auto& m:markers)if(m.query)m.query->Release();if(disjoint)disjoint->Release();if(start)start->Release();if(end)end->Release();}
    };
    std::array<Slot,16> slots_{};
    Slot* active_{};
    bool initialized_{},failed_{};
public:
    void poll(ID3D11DeviceContext* ctx,ProfileTrace& trace) {
        for(auto& s:slots_)if(s.pending) {
            D3D11_QUERY_DATA_TIMESTAMP_DISJOINT d{}; UINT64 a{},b{};
            const UINT flags=D3D11_ASYNC_GETDATA_DONOTFLUSH;
            if(ctx->GetData(s.disjoint,&d,sizeof(d),flags)!=S_OK ||
               ctx->GetData(s.start,&a,sizeof(a),flags)!=S_OK ||
               ctx->GetData(s.end,&b,sizeof(b),flags)!=S_OK)continue;
            bool ready=true;
            for(unsigned i=0;i<s.markerCount;++i)
                if(ctx->GetData(s.markers[i].query,&s.markers[i].value,sizeof(UINT64),flags)!=S_OK){ready=false;break;}
            if(!ready)continue;
            const bool valid=!d.Disjoint&&d.Frequency&&b>=a;
            trace.record(valid?"gpu_frame_elapsed":"gpu_disjoint",s.frame,s.cpuStart,
                valid?double(b-a)*1000.0/double(d.Frequency):-1.0);
            if(s.markerOverflow)trace.record("gpu_marker_capacity",s.frame,s.cpuStart,-1);
            if(valid)for(unsigned i=0;i<s.markerCount;++i)if(!s.markers[i].opening) {
                auto& finish=s.markers[i];
                for(unsigned j=i;j>0;--j) {
                    auto& start=s.markers[j-1];
                    if(start.opening&&start.name==finish.name) {
                        if(finish.value>=start.value)trace.record(start.name.c_str(),s.frame,start.cpuStart,
                            double(finish.value-start.value)*1000.0/double(d.Frequency));
                        break;
                    }
                }
            }
            s.pending=false;
        }
    }
    void begin(ID3D11Device* dev,ID3D11DeviceContext* ctx,ProfileTrace& trace,uint64_t frame) {
        if(!trace.enabled()||failed_||active_)return;
        if(!initialized_) {
            for(auto& s:slots_) {
                D3D11_QUERY_DESC d{D3D11_QUERY_TIMESTAMP_DISJOINT,0};
                if(FAILED(dev->CreateQuery(&d,&s.disjoint))){failed_=true;break;}
                d.Query=D3D11_QUERY_TIMESTAMP;
                if(FAILED(dev->CreateQuery(&d,&s.start))||FAILED(dev->CreateQuery(&d,&s.end))){failed_=true;break;}
            }
            initialized_=true;
            if(failed_){trace.record("gpu_query_unavailable",frame,profileClockMs(),-1);return;}
        }
        poll(ctx,trace);
        for(auto& s:slots_)if(!s.pending) {
            active_=&s;s.frame=frame;s.cpuStart=profileClockMs();s.markerCount=0;s.markerOverflow=false;
            ctx->Begin(s.disjoint);ctx->End(s.start);return;
        }
        trace.record("gpu_query_ring_full",frame,profileClockMs(),-1);
    }
    void marker(ID3D11Device* dev,ID3D11DeviceContext* ctx,const char* name,bool opening) {
        if(!active_)return;
        if(active_->markerCount>=active_->markers.size()){active_->markerOverflow=true;return;}
        auto& m=active_->markers[active_->markerCount];
        if(!m.query) {
            D3D11_QUERY_DESC d{D3D11_QUERY_TIMESTAMP,0};
            if(FAILED(dev->CreateQuery(&d,&m.query)))return;
        }
        m.name=name;m.opening=opening;m.cpuStart=profileClockMs();
        ctx->End(m.query);++active_->markerCount;
    }
    void end(ID3D11DeviceContext* ctx) {
        if(!active_)return;
        ctx->End(active_->end);ctx->End(active_->disjoint);
        active_->pending=true;active_=nullptr;
    }
};
}
