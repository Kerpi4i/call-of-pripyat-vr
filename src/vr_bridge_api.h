#pragma once
#include <stdint.h>
#define COPVR_ABI 2u
struct CopVrEye {
    float position[3],direction[3],up[3];
    float left,right,upAngle,downAngle;
};
struct CopVrFrame { uint32_t abi,render,focused,menu; struct CopVrEye eyes[2]; };
typedef int (*CopVrInitialize)(void* device,uint32_t width,uint32_t height);
typedef int (*CopVrBegin)(struct CopVrFrame* frame);
typedef int (*CopVrCopyEye)(uint32_t eye,void* texture);
typedef int (*CopVrEnd)(void);
typedef void (*CopVrShutdown)(void);
typedef const char* (*CopVrError)(void);

#define COPVR_CONTROLS_ABI 1u
struct CopVrHand {
    uint32_t valid;
    float trigger,squeeze;
    float aimPosition[3],aimDirection[3],aimUp[3],gripPosition[3];
};
struct CopVrControls { uint32_t abi,active; uint64_t timestampMs; struct CopVrHand hands[2]; };
typedef int (*CopVrGetControls)(struct CopVrControls* controls);
// Optional extension: keep the original controls ABI intact for older clients.
#define COPVR_GRIP_ABI 1u
struct CopVrGripPose { uint32_t valid; float direction[3],up[3]; };
struct CopVrGrips { uint32_t abi,active; uint64_t timestampMs; struct CopVrGripPose hands[2]; };
typedef int (*CopVrGetGrips)(struct CopVrGrips* grips);
// Optional copvr_get_ui_controls export uses the same layout. It is active
// only for an in-game overlay, never for gameplay input or the main menu.
typedef int (*CopVrPulse)(uint32_t hand,float amplitude,uint32_t milliseconds);

#define COPVR_POINTER_ABI 1u
struct CopVrPointer { uint32_t abi,active; uint64_t timestampMs; float x,y,trigger; uint32_t hand; };
typedef int (*CopVrGetPointer)(struct CopVrPointer* pointer);

#define COPVR_HEAD_ABI 1u
struct CopVrHead { uint32_t abi,active; uint64_t timestampMs; float position[3],direction[3],up[3]; };
typedef int (*CopVrGetHead)(struct CopVrHead* head);

#define COPVR_NAV_ABI 1u
enum CopVrButton { COPVR_A=1, COPVR_B=2, COPVR_X=4, COPVR_Y=8, COPVR_LSTICK=16, COPVR_RSTICK=32, COPVR_MENU=64, COPVR_USE=128, COPVR_HOLSTER=256, COPVR_RELOAD=512 };
struct CopVrNavigation { uint32_t abi,active; uint64_t timestampMs; float moveX,moveY,turnX,scrollY; uint32_t buttons,tracked; };
typedef int (*CopVrGetNavigation)(struct CopVrNavigation* navigation);
typedef int (*CopVrCopyUI)(void* texture);

// Optional extension. Calibration only changes vertical origin, never yaw/XZ.
#define COPVR_HEIGHT_ABI 1u
struct CopVrHeight { uint32_t abi,active; uint64_t timestampMs; float standingEyeHeight; uint32_t floorValid,serial; };
typedef int (*CopVrGetHeight)(struct CopVrHeight* height);
typedef int (*CopVrCalibrateHeight)(void);
