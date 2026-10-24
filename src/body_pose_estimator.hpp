#pragma once
#include "beam_math.hpp"
#include <algorithm>
namespace copvr {
struct EstimatedBodyPose { Vec3 waistOrigin{},pelvis{},chestCenter{},chestUp{0,1,0}; float crouch=0; };
class BodyPoseEstimator {
    bool valid_=false,walking_=false; Vec3 anchor_{}; float uprightTime_=0;
public:
    void reset() { valid_=false; walking_=false; uprightTime_=0; }
    EstimatedBodyPose update(Vec3 head,Vec3 headForward,Vec3 headUp,float yaw,float eyeHeight,float dt,bool recalibrate=false) {
        EstimatedBodyPose out;
        if(!std::isfinite(head.length())||!std::isfinite(yaw)) { reset(); return out; }
        eyeHeight=std::clamp(eyeHeight,1.2f,2.2f);dt=std::clamp(dt,0.f,.05f);
        const Vec3 forward{std::sin(yaw),0,std::cos(yaw)};
        const Vec3 neck=head-unit(headUp)*.10f-unit(headForward)*.08f;
        if(!valid_||recalibrate) { anchor_={head.x,0,head.z}; valid_=true; walking_=false; uprightTime_=0; }
        Vec3 neutralNeck=anchor_-forward*.08f;
        Vec3 lean{neck.x-neutralNeck.x,0,neck.z-neutralNeck.z};
        float horizontal=lean.length();
        // Follow real upright room-scale steps, but retain an anchor while bending.
        if(std::abs(head.y)<.025f) {
            if(!walking_) { if(horizontal>.20f) uprightTime_+=dt;else uprightTime_=0; }
            if(uprightTime_>.25f) walking_=true;
        } else { uprightTime_=0;walking_=false; }
        if(walking_&&horizontal<=.065f) { walking_=false;uprightTime_=0; }
        if(walking_&&horizontal>.06f) {
            anchor_=anchor_+lean*((1-std::exp(-4*dt))*(horizontal-.06f)/horizontal);
        }
        const float hipOffset=eyeHeight*.45f,spine=hipOffset-.10f;
        neutralNeck=anchor_-forward*.08f;
        lean={neck.x-neutralNeck.x,0,neck.z-neutralNeck.z};horizontal=lean.length();
        const float maxLean=spine*.9f;
        if(horizontal>maxLean) { anchor_=anchor_+lean*((horizontal-maxLean)/horizontal);neutralNeck=anchor_-forward*.08f;lean={neck.x-neutralNeck.x,0,neck.z-neutralNeck.z};horizontal=lean.length(); }
        const float leanDrop=spine-std::sqrt(std::max(.001f,spine*spine-horizontal*horizontal));
        const float maxCrouch=std::max(.1f,eyeHeight-hipOffset-.30f);
        const float crouch=std::clamp(-.10f-leanDrop-neck.y,0.f,maxCrouch);
        const float rise=std::max(0.f,head.y);
        Vec3 hip{neutralNeck.x,rise-hipOffset-crouch,neutralNeck.z};
        const float chestFraction=std::clamp((hipOffset-.29f)/spine,.4f,.85f);
        out.waistOrigin={anchor_.x,rise-crouch,anchor_.z};
        out.chestCenter=hip+(neck-hip)*chestFraction+forward*.08f;
        // Deep crouch shortens the remaining spine: keep the two fixed-size
        // interaction rows separated instead of folding quick slots into the chest zone.
        if(crouch>=maxCrouch-.001f) out.waistOrigin.y=std::min(out.waistOrigin.y,out.chestCenter.y-.23f+.535f);
        out.pelvis=hip;
        out.chestUp=unit(unit(neck-hip)*.7f+Vec3{0,.3f,0});
        out.crouch=crouch; return out;
    }
};
}
