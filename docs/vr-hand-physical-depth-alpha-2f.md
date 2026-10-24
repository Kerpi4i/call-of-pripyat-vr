# Physical depth for VR HUD hands, alpha 2f

Diagnosis: hud_transform_helper preserves the VR eye projection but still calls rmNear. rmNear maps HUD depth into [0,0.02] while hand panels draw at physical [0,1] depth; a hand therefore occludes its own panel regardless of physical ordering.

Use rmNormal in VR for HUD geometry, retaining rmNear for desktop. VR hands/items, dorsal panels and world geometry now share physical depth; retain depth testing on both panel shaders. Panel poses and offsets are unchanged. A physically closer object can still occlude a panel naturally. Near-wall weapon/hand occlusion can change because the desktop always-near behavior is removed.

Validation: Game target build; diff/patch checks. Headset verification required. No game launch.
