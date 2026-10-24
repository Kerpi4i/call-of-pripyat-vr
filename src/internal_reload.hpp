#pragma once
namespace copvr {
enum class InternalKind { None, Tube, Break, Rocket, Rotary, Drum };
struct InternalReload {
    bool initialized=false,needsPump=false,rearReached=false;
    unsigned live=0,spent=0,cursor=0;
    float pump=0,angle=0;
    void init(int rounds,InternalKind kind) {
        if(initialized)return;
        initialized=true;
        if(kind==InternalKind::Drum)live=rounds>=6?63u:rounds>0?(1u<<rounds)-1u:0u;
        if(kind==InternalKind::Break)live=rounds>=2?3u:rounds==1?1u:0u;
    }
    bool canFire(InternalKind kind) const {
        return (kind==InternalKind::Tube||kind==InternalKind::Rotary)?!needsPump&&pump<.08f:
            kind==InternalKind::Break?angle<.05f:
            kind==InternalKind::Drum?!needsPump&&angle<.03f&&drumIndexed()&&!!(live&(1u<<cursor)):true;
    }
    void shot(InternalKind kind) {
        if(kind==InternalKind::Tube){needsPump=true;rearReached=false;spent=1;}
        if(kind==InternalKind::Drum) {live&=~(1u<<cursor);if(live)drumNext(false);else cursor=(cursor+1)%6;}
        if(kind==InternalKind::Break&&live) {
            const unsigned chamber=live&(~live+1u);live&=~chamber;spent|=chamber;
        }
    }
    bool pumpTo(float value,int rounds) {
        pump=value<0?0:value>1?1:value;
        if(pump>=.9f)rearReached=true;
        if(rearReached&&pump<=.08f){needsPump=false;rearReached=false;return true;}
        return false;
    }
    bool canLoadTube(bool requireOpen) const {return !requireOpen||(pump>=.9f&&rearReached);}
    void tubeLoaded(int previousRounds) { if(previousRounds==0)needsPump=true; }
    void drumNext(bool empty) {
        for(unsigned n=1;n<=6;++n) {
            const unsigned c=(cursor+n)%6;
            if(empty?!((live>>c)&1u):!!((live>>c)&1u)){cursor=c;return;}
        }
    }
    bool drumIndexed() const {
        float gap=pump-float(cursor)/6.f;if(gap<0)gap=-gap;
        return gap<.005f||gap>.995f;
    }
    void drumAnimate(float dt) {
        const float target=float(cursor)/6.f;float gap=target-pump;if(gap<0)gap+=1.f;
        if(gap<.001f||gap>.999f){pump=target;return;}
        const float step=dt>0?dt:0;pump+=gap<step?gap:step;if(pump>=1.f)pump-=1.f;
    }
    bool drumCanLoad() const {return drumIndexed()&&needsPump&&angle>=.60f&&cursor<6&&!(live&(1u<<cursor));}
    bool drumLoad() {if(!drumCanLoad())return false;live|=1u<<cursor;if(live==63u)cursor=(cursor+1)%6;else drumNext(true);return true;}
    void drumToggle() {needsPump=!needsPump;if(!needsPump&&!(live&(1u<<cursor)))drumNext(false);else if(needsPump&&(live&(1u<<cursor)))drumNext(true);}
    bool canLoad(int chamber) const {
        return angle>=.40f&&chamber>=0&&chamber<2&&!((live|spent)&(1u<<chamber));
    }
    bool load(int chamber) {
        if(!canLoad(chamber))return false;
        live|=1u<<chamber;return true;
    }
    unsigned eject() { const auto result=spent;spent=0;return result; }
};
}
