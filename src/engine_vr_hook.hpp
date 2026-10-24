#pragma once
#include "vr_bridge_api.h"
#include "vr_frame_mode.hpp"
#include "vr_optics_api.hpp"
#include "vr_optics_limits.hpp"
#if defined(XR_PLATFORM_WINDOWS)
namespace copvr_engine {
struct Hook {
    HMODULE library{}; bool attempted{},enabled{},hadFocus{},renderingStereo{},loadingDraw{};
    unsigned long long frames{};
    CopVrInitialize initialize{}; CopVrBegin begin{}; CopVrCopyEye copy{};
    CopVrEnd end{}; CopVrShutdown shutdown{}; CopVrError error{};
    CopVrCopyUI copyUI{},copyOptics{};
    CopVrGetOptics getOptics{}; using SetOptics=void(*)(int); SetOptics setOptics{};
    using SetUI=void(*)(int,int); SetUI setUI{};
    using GetDevice=void*(*)(); using GetBuffer=void*(*)(); using SyncCamera=void(*)(int); using ReleaseBuffer=void(*)(void*);
    GetDevice getUITexture{}; using ReleaseUI=void(*)(); ReleaseUI releaseUI{};
    GetDevice getDevice{}; GetBuffer getBuffer{}; SyncCamera syncCamera{}; ReleaseBuffer releaseBuffer{};
    using IsIntro=int(*)(); IsIntro isIntro{};
    IsIntro isUI{}; using UpdatePointer=void(*)(); UpdatePointer updatePointer{},updateUINavigation{};
    ~Hook() { close(); }
    bool requested() const { return strstr(Core.Params,"-copvr")!=nullptr; }
    template<class T> T proc(HMODULE module,const char* name) { return reinterpret_cast<T>(GetProcAddress(module,name)); }
    void close() {
        if(releaseUI) releaseUI();
        releaseUI=nullptr; setUI=nullptr; getUITexture=nullptr;
        if(shutdown&&library) shutdown();
        if(library) FreeLibrary(library);
        library=nullptr; enabled=false;
    }
    bool load() {
        if(attempted) return enabled;
        attempted=true;
        if(!requested()) return false;
        auto renderer=GetModuleHandleA("xrRender_R4.dll");
        if(!renderer) { Msg("! CoPVR: D3D11 renderer_r4 is required"); return false; }
        getDevice=proc<GetDevice>(renderer,"copvr_get_device"); getBuffer=proc<GetBuffer>(renderer,"copvr_get_backbuffer");
        syncCamera=proc<SyncCamera>(renderer,"copvr_sync_camera");
        setUI=proc<SetUI>(renderer,"copvr_set_ui"); getUITexture=proc<GetDevice>(renderer,"copvr_get_ui_texture");
        releaseUI=proc<ReleaseUI>(renderer,"copvr_release_ui");
        setOptics=proc<SetOptics>(renderer,"copvr_set_optics_pass");
        copyOptics=proc<CopVrCopyUI>(renderer,"copvr_copy_optics");
        releaseBuffer=proc<ReleaseBuffer>(renderer,"copvr_release_backbuffer");
        if(auto game=GetModuleHandleA("xrGame.dll")) {
            getOptics=proc<CopVrGetOptics>(game,"copvr_get_binocular_view");
            isIntro=proc<IsIntro>(game,"copvr_is_intro");
            isUI=proc<IsIntro>(game,"copvr_is_ui");
            updatePointer=proc<UpdatePointer>(game,"copvr_update_pointer");
            updateUINavigation=proc<UpdatePointer>(game,"copvr_update_ui_navigation");
        }
        if(!getDevice||!getBuffer||!syncCamera||!releaseBuffer) { Msg("! CoPVR: renderer exports are missing"); return false; }
        string_path path; GetModuleFileNameA(nullptr,path,sizeof(path));
        auto slash=strrchr(path,'\\'); if(!slash) return false;
        *(slash+1)=0; xr_strcat(path,"copvr_bridge.dll");
        library=LoadLibraryExA(path,nullptr,LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR|LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
        if(!library) { Msg("! CoPVR: bridge load failed: %lu",GetLastError()); return false; }
        initialize=proc<CopVrInitialize>(library,"copvr_initialize"); begin=proc<CopVrBegin>(library,"copvr_begin");
        copy=proc<CopVrCopyEye>(library,"copvr_copy_eye"); end=proc<CopVrEnd>(library,"copvr_end");
        copyUI=proc<CopVrCopyUI>(library,"copvr_copy_ui");
        shutdown=proc<CopVrShutdown>(library,"copvr_shutdown"); error=proc<CopVrError>(library,"copvr_error");
        if(!initialize||!begin||!copy||!copyUI||!setUI||!getUITexture||!releaseUI||!end||!shutdown||!error) { Msg("! CoPVR: incompatible bridge exports"); close(); return false; }
        enabled=initialize(getDevice(),Device.dwWidth,Device.dwHeight)>0;
        if(!enabled) { Msg("! CoPVR: initialization failed: %s",error()); close(); }
        else { Msg("* CoPVR: initialized on engine D3D11 device, %ux%u per eye",Device.dwWidth,Device.dwHeight); FlushLog(); }
        return enabled;
    }
    void failed(const char* stage) { Msg("! CoPVR %s: %s",stage,error?error():"unknown error"); close(); }
};
inline Hook& hook() { static Hook value; return value; }
}
#endif
