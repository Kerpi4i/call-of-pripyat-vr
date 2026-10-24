#pragma once
#include "game_vr_scope_test_settings.hpp"
#include "xrGame/Actor.h"
#include "xrGame/Inventory.h"
#include "xrGame/Weapon.h"
#include "xrGame/WeaponAmmo.h"
#include <map>
#include <string>
#include "xrGame/Level.h"
#include "xrServerEntities/xrServer_Objects_ALife_Items.h"
#include "xrNetServer/NET_Messages.h"
namespace copvr_game {
inline void issueScopeTestKit(CActor& actor,bool& issued) {
    if(issued||!scopeTestActive()||!actor.g_Alive()||Device.Paused()||Level().CurrentControlEntity()!=&actor||!Level().IsServer()) return;
    issued=true; unsigned count=0; const bool integrated=scopeTestMode()==2;
    if(scopeTestMode()>=3) {
        const bool launchers=scopeTestMode()==4,worn=scopeTestMode()==5;
        std::map<std::string,unsigned> ammunition;
        auto queue=[&](CSE_Abstract* entity) {
            NET_Packet packet; entity->Spawn_Write(packet,TRUE); Level().Send(packet,net_flags(TRUE)); F_entity_Destroy(entity);
        };
        for(auto section:pSettings->sections()) {
            const char* name=section->Name.c_str();
            if(strncmp(name,"wpn_",4)||!pSettings->line_exist(name,"hud")||!pSettings->line_exist(name,"class")) continue;
            const char* cls=pSettings->r_string(name,"class");
            if(strncmp(cls,"WP_",3)) continue;
            const bool abakan=pSettings->line_exist(name,"hud")&&!xr_strcmp(pSettings->r_string(name,"hud"),"wpn_abakan_hud");
            const int launcherStatus=pSettings->line_exist(name,"grenade_launcher_status")?pSettings->r_s32(name,"grenade_launcher_status"):0;
            if(launchers&&launcherStatus==0&&!abakan)continue;
            auto entity=Level().spawn_item(name,actor.Position(),actor.ai_location().level_vertex_id(),actor.ID(),true);
            auto weapon=smart_cast<CSE_ALifeItemWeapon*>(entity);
            if(!weapon) { F_entity_Destroy(entity); continue; }
            if(launchers){weapon->m_addon_flags.set(CSE_ALifeItemWeapon::eWeaponAddonGrenadeLauncher,TRUE);if(abakan)weapon->m_upgrades.emplace_back("up_fiftha_abakan");}
            if(worn)weapon->m_fCondition=.05f;
            bool owned=false;
            for(auto item:actor.inventory().m_all) if(!xr_strcmp(item->object().cNameSect().c_str(),name)) {
                auto existing=smart_cast<CWeapon*>(item);if(worn&&existing)existing->SetCondition(.05f);
                if(!launchers||(existing&&existing->IsGrenadeLauncherAttached()))owned=true;
            }
            if(owned) F_entity_Destroy(entity); else { queue(entity); ++count; }
            const char* ammoKey=launchers?"grenade_class":"ammo_class";
            if(xr_strcmp(cls,"WP_KNIFE")&&xr_strcmp(cls,"WP_BINOC")&&pSettings->line_exist(name,ammoKey)) {
                const char* list=pSettings->r_string(name,ammoKey); string256 ammo;
                for(int i=0;i<_GetItemCount(list);++i) { _GetItem(list,i,ammo); if(pSettings->section_exist(ammo)) ammunition[ammo]+=100; }
            }
        }
        if(!launchers)for(const char* name:{"grenade_f1","grenade_rgd5"}) {
            if(!pSettings->section_exist(name)) continue;
            bool owned=false;
            for(auto item:actor.inventory().m_all) if(!xr_strcmp(item->object().cNameSect().c_str(),name)) { owned=true;break; }
            if(!owned) queue(Level().spawn_item(name,actor.Position(),actor.ai_location().level_vertex_id(),actor.ID(),true));
        }
        // Shared calibres use one inventory pool: reserve 100 rounds per compatible weapon.
        for(const auto& entry:ammunition) {
            unsigned have=0;
            for(auto item:actor.inventory().m_all) if(!xr_strcmp(item->object().cNameSect().c_str(),entry.first.c_str()))
                if(auto ammo=smart_cast<CWeaponAmmo*>(item)) have+=ammo->m_boxCurr;
            unsigned remaining=entry.second>have?entry.second-have:0;
            while(remaining) {
                auto entity=Level().spawn_item(entry.first.c_str(),actor.Position(),actor.ai_location().level_vertex_id(),actor.ID(),true);
                auto ammo=smart_cast<CSE_ALifeItemAmmo*>(entity);
                if(!ammo||!ammo->m_boxSize) { F_entity_Destroy(entity); break; }
                const unsigned amount=_min(remaining,unsigned(ammo->m_boxSize)); ammo->a_elapsed=u16(amount);
                queue(entity); remaining-=amount;
            }
        }
        Msg("* CoPVR weapon test mode=%d: queued %u weapons, %u ammunition types",scopeTestMode(),count,unsigned(ammunition.size()));
        return;
    }
    for(auto section:pSettings->sections()) {
        const char* name=section->Name.c_str();
        if(strncmp(name,"wpn_",4)||!pSettings->line_exist(name,"hud")||!pSettings->line_exist(name,"scope_status")) continue;
        const char* hud=pSettings->r_string(name,"hud");
        string256 profile; xr_strcpy(profile,"scope_vr_"); xr_strcat(profile,hud);
        auto ini=equipmentConfig(); if(!ini||!ini->section_exist(profile)) continue;
        int status=pSettings->r_s32(name,"scope_status");
        const bool upgradeAK=status==0&&!xr_strcmp(hud,"wpn_ak74_hud");
        if(integrated) { if(status!=1) continue; }
        else if(!pSettings->line_exist(name,"scopes_sect")||(status!=2&&!upgradeAK)) continue;
        bool owned=false;
        for(auto item:actor.inventory().m_all) {
            auto weapon=smart_cast<CWeapon*>(item);
            if(weapon&&!xr_strcmp(weapon->cNameSect().c_str(),name)&&weapon->IsScopeAttached()) { owned=true; break; }
        }
        if(owned) continue;
        auto entity=Level().spawn_item(name,actor.Position(),actor.ai_location().level_vertex_id(),actor.ID(),true);
        auto weapon=smart_cast<CSE_ALifeItemWeapon*>(entity);
        if(!weapon) { F_entity_Destroy(entity); continue; }
        if(!integrated) weapon->m_addon_flags.set(CSE_ALifeItemWeapon::eWeaponAddonScope,TRUE);
        if(!integrated&&upgradeAK) weapon->m_upgrades.emplace_back("up_fiftha_ak74");
        NET_Packet packet; entity->Spawn_Write(packet,TRUE); Level().Send(packet,net_flags(TRUE));
        F_entity_Destroy(entity); ++count;
        Msg("* CoPVR scope test: queued %s with scope",name);
    }
    Msg("* CoPVR scope test: queued %u scoped weapons; original save untouched",count);
}
}
