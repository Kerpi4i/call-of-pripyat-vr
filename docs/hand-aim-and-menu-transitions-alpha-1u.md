# Hand aiming and menu transition fixes, alpha 1u

## Changes
- Hide the HUD quick-slot icons/labels drawn by DrawMainIndicatorsForInventory in VR. Hide and disable the editable quick reference list in actor menu Draw; keep saved assignments and body quick-slot access.
- Remove both widescreen sidepanel auto_static nodes from PDA XML override.
- SetUICursorPosition records previous UI position and avoids desktop cursor warping in VR. Native mouse-held polling obtains the virtual Touch UI trigger through an xrGame export, including UI core sliders, buttons and scroll views.
- PDA map hint routing and map dragging recognize the virtual held trigger. Grip sends secondary mouse click for map properties; existing right-stick wheel zoom/scroll remains.
- Process X/B/Y menu toggles before selecting frame composition mode. If pointer input changes the composition mode after OpenXR frame begin, finish that transition frame without submitting stale eye/UI images. Next frame uses the correct composition. Headset validation required for transient flat/body flash.
- Replace HUD world ray from HMD with a valid available Touch aim pose. Preserve the last used hand, use another available tracked hand if needed, and clear query when no valid hand is available. Fresh empty-hand trigger chooses that hand, refreshes ray and actor interaction target before forwarding the native use action. Native distance/occlusion and script/tutorial use handling remain.

## Validation
Build Game target and frame-mode headless unit tests. No game/test launch without user command. Quest Link check still required: all menu transitions, PDA click/drag/zoom/context menu, NPC/door/switch activation with both empty hands, sleep regression.
