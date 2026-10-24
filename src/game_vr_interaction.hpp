#pragma once
#include "game_vr_controls.hpp"
namespace copvr_game {
struct InteractionHand { int side=1; bool available[2]={true,true}; };
inline InteractionHand& interactionHand() { static InteractionHand value; return value; }
inline bool interactionRay(const Fmatrix& camera,Fvector& origin,Fvector& direction) {
    CopVrControls controls{};
    if(!sample(controls)) return false;
    auto& state=interactionHand();
    for(int side:{state.side,1-state.side}) {
        if(!state.available[side]) continue;
        Fmatrix aim; Fvector grip;
        if(hand(controls,side,camera,aim,grip)) { origin=aim.c; direction=aim.k; return true; }
    }
    return false; // Never aim world interactions from the head when tracking is unavailable.
}
}
