#include "openxr_input.hpp"
#include <cstring>
#include <string>
namespace copvr {
void OpenXRInput::check(XrResult result,const char* operation) {
    if(XR_FAILED(result)) throw std::runtime_error(std::string(operation)+": "+std::to_string(result));
}
XrPath OpenXRInput::path(const char* name) const {
    XrPath value{}; check(proc<PFN_xrStringToPath>("xrStringToPath")(instance_,name,&value),name); return value;
}
XrAction OpenXRInput::action(const char* name,XrActionType type) {
    XrActionCreateInfo info{XR_TYPE_ACTION_CREATE_INFO};
    std::strncpy(info.actionName,name,sizeof(info.actionName)-1);
    std::strncpy(info.localizedActionName,name,sizeof(info.localizedActionName)-1);
    info.actionType=type; info.countSubactionPaths=2; info.subactionPaths=paths_.data();
    XrAction result{};
    check(proc<PFN_xrCreateAction>("xrCreateAction")(actions_,&info,&result),"xrCreateAction"); return result;
}
OpenXRInput::OpenXRInput(PFN_xrGetInstanceProcAddr get,XrInstance instance,XrSession session,XrSpace reference)
    :get_(get),instance_(instance),session_(session),reference_(reference) {
    try {
        paths_={path("/user/hand/left"),path("/user/hand/right")};
        XrActionSetCreateInfo set{XR_TYPE_ACTION_SET_CREATE_INFO};
        std::strncpy(set.actionSetName,"copvr",sizeof(set.actionSetName)-1);
        std::strncpy(set.localizedActionSetName,"Call of Pripyat VR",sizeof(set.localizedActionSetName)-1);
        check(proc<PFN_xrCreateActionSet>("xrCreateActionSet")(instance_,&set,&actions_),"xrCreateActionSet");
        grip_=action("grip_pose",XR_ACTION_TYPE_POSE_INPUT);
        aim_=action("aim_pose",XR_ACTION_TYPE_POSE_INPUT);
        squeeze_=action("squeeze",XR_ACTION_TYPE_FLOAT_INPUT);
        trigger_=action("trigger",XR_ACTION_TYPE_FLOAT_INPUT);
        haptic_=action("haptic",XR_ACTION_TYPE_VIBRATION_OUTPUT);
        stick_=action("thumbstick",XR_ACTION_TYPE_VECTOR2F_INPUT);
        primary_=action("primary",XR_ACTION_TYPE_BOOLEAN_INPUT);
        secondary_=action("secondary",XR_ACTION_TYPE_BOOLEAN_INPUT);
        stickClick_=action("stick_click",XR_ACTION_TYPE_BOOLEAN_INPUT);
        menu_=action("menu",XR_ACTION_TYPE_BOOLEAN_INPUT);
        std::vector<XrActionSuggestedBinding> bindings;
        const char* hands[]={"/user/hand/left","/user/hand/right"};
        for(int h=0;h<2;++h) {
            const std::string root=hands[h];
            bindings.push_back({grip_,path((root+"/input/grip/pose").c_str())});
            bindings.push_back({aim_,path((root+"/input/aim/pose").c_str())});
            bindings.push_back({squeeze_,path((root+"/input/squeeze/value").c_str())});
            bindings.push_back({trigger_,path((root+"/input/trigger/value").c_str())});
            bindings.push_back({haptic_,path((root+"/output/haptic").c_str())});
            bindings.push_back({stick_,path((root+"/input/thumbstick").c_str())});
            bindings.push_back({stickClick_,path((root+"/input/thumbstick/click").c_str())});
            bindings.push_back({primary_,path((root+(h==0?"/input/x/click":"/input/a/click")).c_str())});
            bindings.push_back({secondary_,path((root+(h==0?"/input/y/click":"/input/b/click")).c_str())});
        }
        bindings.push_back({menu_,path("/user/hand/left/input/menu/click")});
        XrInteractionProfileSuggestedBinding suggested{XR_TYPE_INTERACTION_PROFILE_SUGGESTED_BINDING};
        suggested.interactionProfile=path("/interaction_profiles/oculus/touch_controller");
        suggested.countSuggestedBindings=static_cast<uint32_t>(bindings.size()); suggested.suggestedBindings=bindings.data();
        check(proc<PFN_xrSuggestInteractionProfileBindings>("xrSuggestInteractionProfileBindings")(instance_,&suggested),"Touch bindings");
        for(int h=0;h<2;++h) {
            XrActionSpaceCreateInfo info{XR_TYPE_ACTION_SPACE_CREATE_INFO};
            info.subactionPath=paths_[h]; info.poseInActionSpace.orientation.w=1;
            info.action=grip_;
            check(proc<PFN_xrCreateActionSpace>("xrCreateActionSpace")(session_,&info,&grips_[h]),"grip space");
            info.action=aim_;
            check(proc<PFN_xrCreateActionSpace>("xrCreateActionSpace")(session_,&info,&aims_[h]),"aim space");
        }
        XrReferenceSpaceCreateInfo view{XR_TYPE_REFERENCE_SPACE_CREATE_INFO};
        view.referenceSpaceType=XR_REFERENCE_SPACE_TYPE_VIEW; view.poseInReferenceSpace.orientation.w=1;
        check(proc<PFN_xrCreateReferenceSpace>("xrCreateReferenceSpace")(session_,&view,&head_),"view space");
        view.referenceSpaceType=XR_REFERENCE_SPACE_TYPE_STAGE;
        // STAGE is optional; LOCAL still supports relative physical crouching.
        if(XR_FAILED(proc<PFN_xrCreateReferenceSpace>("xrCreateReferenceSpace")(session_,&view,&floor_))) floor_=XR_NULL_HANDLE;
        XrSessionActionSetsAttachInfo attach{XR_TYPE_SESSION_ACTION_SETS_ATTACH_INFO};
        attach.countActionSets=1; attach.actionSets=&actions_;
        check(proc<PFN_xrAttachSessionActionSets>("xrAttachSessionActionSets")(session_,&attach),"attach actions");
    } catch(...) {
        // Destroy all successfully created children on partial initialization.
        for(auto s:grips_) if(s) { PFN_xrVoidFunction fn{}; get_(instance_,"xrDestroySpace",&fn); if(fn) reinterpret_cast<PFN_xrDestroySpace>(fn)(s); }
        for(auto s:aims_) if(s) { PFN_xrVoidFunction fn{}; get_(instance_,"xrDestroySpace",&fn); if(fn) reinterpret_cast<PFN_xrDestroySpace>(fn)(s); }
        if(head_) { PFN_xrVoidFunction fn{}; get_(instance_,"xrDestroySpace",&fn); if(fn) reinterpret_cast<PFN_xrDestroySpace>(fn)(head_); }
        if(floor_) { PFN_xrVoidFunction fn{}; get_(instance_,"xrDestroySpace",&fn); if(fn) reinterpret_cast<PFN_xrDestroySpace>(fn)(floor_); }
        if(actions_) { PFN_xrVoidFunction fn{}; get_(instance_,"xrDestroyActionSet",&fn); if(fn) reinterpret_cast<PFN_xrDestroyActionSet>(fn)(actions_); }
        throw;
    }
}
OpenXRInput::~OpenXRInput() {
    PFN_xrVoidFunction fn{}; get_(instance_,"xrDestroySpace",&fn);
    if(fn) {
        auto destroy=reinterpret_cast<PFN_xrDestroySpace>(fn);
        for(auto s:grips_) if(s) destroy(s);
        for(auto s:aims_) if(s) destroy(s);
        if(head_) destroy(head_);
        if(floor_) destroy(floor_);
    }
    fn=nullptr; get_(instance_,"xrDestroyActionSet",&fn);
    if(fn&&actions_) reinterpret_cast<PFN_xrDestroyActionSet>(fn)(actions_);
}
Pose OpenXRInput::locate(XrSpace space,XrTime time,Vec3* velocity,bool* velocityValid) const {
    XrSpaceVelocity speed{XR_TYPE_SPACE_VELOCITY};
    XrSpaceLocation location{XR_TYPE_SPACE_LOCATION}; location.next=&speed;
    check(proc<PFN_xrLocateSpace>("xrLocateSpace")(space,reference_,time,&location),"xrLocateSpace");
    // Runtime-inferred poses remain usable outside the optical camera cone.
    // TRACKED is stronger than VALID and must not hide an inferred Touch pose.
    constexpr XrSpaceLocationFlags required=XR_SPACE_LOCATION_POSITION_VALID_BIT|XR_SPACE_LOCATION_ORIENTATION_VALID_BIT;
    const auto& p=location.pose;
    if(velocityValid) *velocityValid=(speed.velocityFlags&XR_SPACE_VELOCITY_LINEAR_VALID_BIT)!=0;
    if(velocity) *velocity={speed.linearVelocity.x,speed.linearVelocity.y,speed.linearVelocity.z};
    return {{p.position.x,p.position.y,p.position.z},{p.orientation.x,p.orientation.y,p.orientation.z,p.orientation.w},
        (location.locationFlags&required)==required};
}
void OpenXRInput::pulse(int hand,float amplitude,uint32_t milliseconds) {
    if(hand<0||hand>1||!std::isfinite(amplitude)||amplitude<0||amplitude>1||milliseconds<1||milliseconds>100)
        throw std::runtime_error("Invalid haptic pulse");
    XrHapticActionInfo info{XR_TYPE_HAPTIC_ACTION_INFO}; info.action=haptic_; info.subactionPath=paths_[hand];
    XrHapticVibration vibration{XR_TYPE_HAPTIC_VIBRATION};
    vibration.amplitude=amplitude; vibration.frequency=XR_FREQUENCY_UNSPECIFIED;
    vibration.duration=static_cast<XrDuration>(milliseconds)*1000000;
    check(proc<PFN_xrApplyHapticFeedback>("xrApplyHapticFeedback")(session_,&info,
        reinterpret_cast<const XrHapticBaseHeader*>(&vibration)),"xrApplyHapticFeedback");
}
Frame OpenXRInput::sample(XrTime time,bool focused) {
    Frame result{}; result.focused=focused;
    if(!focused) return result;
    XrActiveActionSet active{actions_,XR_NULL_PATH};
    XrActionsSyncInfo sync{XR_TYPE_ACTIONS_SYNC_INFO}; sync.countActiveActionSets=1; sync.activeActionSets=&active;
    auto status=proc<PFN_xrSyncActions>("xrSyncActions")(session_,&sync);
    if(status==XR_SESSION_NOT_FOCUSED) { result.focused=false; return result; }
    check(status,"xrSyncActions");
    result.head=locate(head_,time);
    if(floor_&&result.head.valid) {
        XrSpaceLocation floorPose{XR_TYPE_SPACE_LOCATION};
        auto status=proc<PFN_xrLocateSpace>("xrLocateSpace")(head_,floor_,time,&floorPose);
        const auto flags=XR_SPACE_LOCATION_POSITION_VALID_BIT|XR_SPACE_LOCATION_POSITION_TRACKED_BIT;
        result.floorEyeHeight=floorPose.pose.position.y;
        result.floorValid=XR_SUCCEEDED(status)&&(floorPose.locationFlags&flags)==flags&&
            std::isfinite(result.floorEyeHeight)&&result.floorEyeHeight>.5f&&result.floorEyeHeight<2.5f;
    }
    for(int h=0;h<2;++h) {
        auto& hand=result.hands[h];
        XrActionStateGetInfo info{XR_TYPE_ACTION_STATE_GET_INFO}; info.subactionPath=paths_[h];
        auto poseActive=[&](XrAction action) {
            info.action=action; XrActionStatePose state{XR_TYPE_ACTION_STATE_POSE};
            check(proc<PFN_xrGetActionStatePose>("xrGetActionStatePose")(session_,&info,&state),"pose action"); return state.isActive;
        };
        if(poseActive(grip_)) hand.grip=locate(grips_[h],time,&hand.velocity,&hand.velocityValid);
        if(poseActive(aim_)) hand.aim=locate(aims_[h],time);
        auto scalar=[&](XrAction action) {
            info.action=action; XrActionStateFloat state{XR_TYPE_ACTION_STATE_FLOAT};
            check(proc<PFN_xrGetActionStateFloat>("xrGetActionStateFloat")(session_,&info,&state),"float action");
            return state.isActive?state.currentState:0.f;
        };
        hand.squeeze=scalar(squeeze_); hand.trigger=scalar(trigger_);
        info.action=stick_; XrActionStateVector2f stick{XR_TYPE_ACTION_STATE_VECTOR2F};
        check(proc<PFN_xrGetActionStateVector2f>("xrGetActionStateVector2f")(session_,&info,&stick),"stick action");
        if(stick.isActive) { hand.stickX=stick.currentState.x; hand.stickY=stick.currentState.y; }
        auto button=[&](XrAction action) {
            info.action=action; XrActionStateBoolean state{XR_TYPE_ACTION_STATE_BOOLEAN};
            check(proc<PFN_xrGetActionStateBoolean>("xrGetActionStateBoolean")(session_,&info,&state),"button action");
            return state.isActive&&state.currentState;
        };
        hand.buttons=(button(primary_)?1u:0u)|(button(secondary_)?2u:0u)|(button(stickClick_)?4u:0u)|(h==0&&button(menu_)?8u:0u);
    }
    return result;
}
}
