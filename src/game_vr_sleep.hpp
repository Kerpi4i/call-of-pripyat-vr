#pragma once
#include "xrGame/ui/UIGameTutorial.h"
extern CUISequencer* g_tutorial;
extern CUISequencer* g_tutorial2;
namespace copvr_game {
inline CUISequencer* sleepTutorial() {
    for(auto tutorial:{g_tutorial,g_tutorial2}) if(tutorial&&tutorial->IsActive()&&tutorial->GetTutorName()&&
        xr_strcmp(tutorial->GetTutorName(),"tutorial_sleep")==0) return tutorial;
    return nullptr;
}
}
