#pragma once
#include "game_vr_controls.hpp"
#include "xrEngine/IGame_Persistent.h"
namespace copvr_game {
inline int scopeTestMode() {
    if(!strstr(Core.Params,"-copvr")||!g_pGamePersistent) return 0;
    auto ini=equipmentConfig(); if(!ini) return 0;
    const char* sections[]={"scope_test","integrated_scope_test","all_weapons_test","underbarrel_test","jam_test"};
    for(int index=0;index<5;++index) {
        const auto section=sections[index];
        if(ini->line_exist(section,"enabled")&&ini->r_bool(section,"enabled")&&ini->line_exist(section,"save_name")&&
            !xr_strcmp(g_pGamePersistent->m_game_params.m_game_or_spawn,ini->r_string(section,"save_name"))) return index+1;
    }
    return 0;
}
inline bool scopeTestActive() { return scopeTestMode()!=0; }
inline bool scopeTestWeight() {
    const int mode=scopeTestMode(); if(!mode) return false;
    auto ini=equipmentConfig(); const char* sections[]={"scope_test","integrated_scope_test","all_weapons_test","underbarrel_test","jam_test"};
    const char* section=sections[mode-1];
    return ini->line_exist(section,"unlimited_weight")&&ini->r_bool(section,"unlimited_weight");
}
}
