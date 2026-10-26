# Development rendering tests

Quest 2 / Link runtime query returned 2080x2096 recommended pixels per eye. The user confirmed Launch-Test-Clarity improved pixelation; the sun-only conservative-shadow experiment did not visibly improve shadow popping.

## Current build

- Launch-Game-VR.cmd, Launch-Test-Clarity.cmd and Launch-Test-Shadows.cmd now all use 2080x2096 eye buffers by default. Desktop vid_mode and native quality settings remain saved separately. Pass -EyeWidth 0 -EyeHeight 0 directly to tools/run-game-vr.ps1 to use saved vid_mode for the eye buffers.
- UI quad dimensions are now 1.875 x 1.125 m, 2 m away, matching the inventory widescreen canvas ratio of 1280:768. Height remains unchanged; width is reduced 6.25%. Pointer hit testing and logical aspect correction use the same ratio. Headset confirmation is pending.
- Local-light shadow-map sizing no longer depends on camera direction in VR. Per-caster monocular occlusion caching is bypassed for VR shadow passes, and HOM is explicitly disabled in those passes. This can cost additional rendering work; headset performance and the visual outcome need verification.
- The conservative sun experiment remains opt-in through -ConservativeShadows; normal launch does not enable it.

## Verification

Native Game target and bridge build passed. ui_aim_filter, vr_render_geometry and vr_ui_layout tests passed. No game was launched during this update.

The user confirmed the local-shadow fix. Keep it unchanged while comparing AA.

## MSAA comparison

Launch-Test-AA-Off.cmd, Launch-Test-MSAA2x.cmd and Launch-Test-MSAA4x.cmd select 1, 2 and 4 geometry samples respectively, through a per-launch renderer override. They do not overwrite r3_msaa in user.ltx. All use the same eye size and shadow fixes. Ordinary launch now respects saved r3_msaa/r3_msaa_alphatest instead of forcing them off. Current saved values are off.

The native combine pass resolves multisampled scene textures before final single-sample output is copied unchanged to OpenXR. A successful build confirms source compatibility, not stereo visual correctness. Renderer startup logs report effective MSAA samples, alpha-test mode and post-AA state.

Start comparison with 2x, then 4x. Check edges of buildings/weapons, foliage, both eyes, scopes/binocular, menus, UI proportions and frame timing. Alpha-test AA remains at the saved value (currently off), so foliage may retain aliasing even if geometry improves. Native post-AA is also currently off. These are separate follow-up comparisons; no claim that MSAA alone fixes all shimmer.

Game/bridge builds and UI geometry tests passed. Both launch overrides passed PrepareOnly checks without starting the game. Headset outcomes for MSAA and the narrower UI panel remain unconfirmed.

Published alpha assets and release source patch are unchanged. No publication is authorized for this branch.
