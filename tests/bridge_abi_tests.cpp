#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "vr_bridge_api.h"
#include <iostream>
#include <cstdlib>
static_assert(sizeof(CopVrEye)==52&&sizeof(CopVrFrame)==120,"VR C ABI layout must match MSVC engine");
static_assert(sizeof(CopVrHand)==60&&sizeof(CopVrControls)==136,"Controls ABI layout");
static_assert(sizeof(CopVrGripPose)==28&&sizeof(CopVrGrips)==72,"Grip extension ABI layout");
static_assert(sizeof(CopVrPointer)==32,"Pointer ABI layout");
static_assert(sizeof(CopVrHead)==56,"Head ABI layout");
static_assert(sizeof(CopVrNavigation)==40,"Navigation ABI layout");
void expect(bool value,const char* reason) { if(!value) { std::cerr<<reason<<'\n'; std::exit(1); } }
template<class T> T symbol(HMODULE library,const char* name) {
    auto result=reinterpret_cast<T>(GetProcAddress(library,name)); expect(result!=nullptr,name); return result;
}
int main() {
    wchar_t path[32768]{}; GetModuleFileNameW(nullptr,path,32768);
    std::wstring location(path); location.resize(location.find_last_of(L"\\")+1); location+=L"copvr_bridge.dll";
    HMODULE library=LoadLibraryExW(location.c_str(),nullptr,LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR|LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
    expect(library!=nullptr,"load app-local bridge");
    auto init=symbol<CopVrInitialize>(library,"copvr_initialize");
    auto begin=symbol<CopVrBegin>(library,"copvr_begin");
    auto copy=symbol<CopVrCopyEye>(library,"copvr_copy_eye");
    auto copyUI=symbol<CopVrCopyUI>(library,"copvr_copy_ui");
    auto end=symbol<CopVrEnd>(library,"copvr_end");
    auto error=symbol<CopVrError>(library,"copvr_error");
    auto shutdown=symbol<CopVrShutdown>(library,"copvr_shutdown");
    auto controls=symbol<CopVrGetControls>(library,"copvr_get_controls");
    auto grips=symbol<CopVrGetGrips>(library,"copvr_get_grips");
    CopVrGrips gripPoses{}; gripPoses.abi=COPVR_GRIP_ABI;
    expect(grips(&gripPoses)==0&&!gripPoses.active,"inactive grips without session");
    gripPoses.abi=999;
    expect(grips(&gripPoses)==-1&&grips(nullptr)==-1,"reject incompatible/null grip extension");
    auto pulse=symbol<CopVrPulse>(library,"copvr_pulse");
    auto pointer=symbol<CopVrGetPointer>(library,"copvr_get_pointer");
    auto head=symbol<CopVrGetHead>(library,"copvr_get_head");
    auto height=symbol<CopVrGetHeight>(library,"copvr_get_height");
    auto calibrate=symbol<CopVrCalibrateHeight>(library,"copvr_calibrate_height");
    CopVrHeight metrics{}; metrics.abi=COPVR_HEIGHT_ABI;
    expect(height(&metrics)==0&&!metrics.active,"height inactive without session");
    metrics.abi=999;
    expect(height(&metrics)==-1&&height(nullptr)==-1,"reject invalid height extension");
    expect(calibrate()==0,"calibration cannot queue without live tracking");
    auto navigation=symbol<CopVrGetNavigation>(library,"copvr_get_navigation");
    CopVrNavigation nav{}; nav.abi=COPVR_NAV_ABI;
    expect(navigation(&nav)==0&&!nav.active,"inactive navigation without session");
    nav.abi=999;
    expect(navigation(&nav)==-1&&navigation(nullptr)==-1,"reject incompatible/null navigation ABI");
    CopVrHead headPose{}; headPose.abi=COPVR_HEAD_ABI;
    expect(head(&headPose)==0&&!headPose.active,"inactive head without session");
    headPose.abi=999;
    expect(head(&headPose)==-1&&head(nullptr)==-1,"reject incompatible/null head ABI");
    CopVrPointer point{}; point.abi=COPVR_POINTER_ABI;
    expect(pointer(&point)==0&&!point.active,"inactive pointer without session");
    point.abi=999;
    expect(pointer(&point)==-1&&pointer(nullptr)==-1,"reject incompatible/null pointer ABI");
    CopVrControls input{}; input.abi=COPVR_CONTROLS_ABI;
    expect(controls(&input)==0&&!input.active,"inactive controls without session");
    input.abi=999;
    expect(controls(&input)==-1&&controls(nullptr)==-1,"reject incompatible/null controls ABI");
    expect(pulse(1,.4f,25)==0,"haptics inactive without session");
    expect(init(nullptr,1280,720)==-1,"reject missing engine device before opening OpenXR");
    expect(error()&&error()[0],"failure includes diagnostic");
    CopVrFrame frame{}; frame.abi=COPVR_ABI;
    expect(begin(&frame)==-1,"reject begin without host");
    expect(begin(nullptr)==-1,"reject null frame");
    expect(copy(0,nullptr)==-1,"reject copy without frame/texture");
    expect(copyUI(nullptr)==-1,"reject UI copy without frame/texture");
    expect(end()==1,"empty end is safe");
    shutdown(); shutdown();
    expect(FreeLibrary(library)!=0,"bridge unload after partial initialization");
    std::cout<<"PASS: C ABI layout, DLL exports, invalid device/frame/copy, diagnostics, idempotent shutdown\n";
}
