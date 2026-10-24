#pragma once
#include "interaction.hpp"
namespace copvr {
// Aim is the runtime's pointing/barrel frame; grip is the contact pose.
// Authored wrist orientation must not pitch the barrel away from this frame.
inline Pose weaponAtPalm(const Pose& aim,const Pose& grip,Vec3 localPalm) {
    if(!aim.valid||!grip.valid) return {};
    return {grip.position-rotate(aim.orientation,localPalm),aim.orientation,true};
}
}
