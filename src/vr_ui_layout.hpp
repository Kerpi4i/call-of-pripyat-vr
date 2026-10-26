#pragma once
namespace copvr {
// Logical/physical UI geometry must not inherit the near-square eye buffer.
// CoP widescreen inventory canvas is 1280x768; retain the existing height.
inline constexpr float uiPanelAspect=1280.f/768.f;
inline constexpr float uiPanelHeight=1.125f;
inline constexpr float uiPanelWidth=uiPanelHeight*uiPanelAspect;
inline constexpr float uiPanelDistance=2.f;
inline constexpr float uiAspectCorrection(float baseWidth,float baseHeight) {
    return (1.f/uiPanelAspect)/(baseHeight/baseWidth);
}
}
