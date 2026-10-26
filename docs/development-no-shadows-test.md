# No dynamic caster shadows diagnostic

Launch-Test-No-Shadows.cmd enables profiling, MSAA 4x and -copvr_no_dynamic_shadows. It does not enable the atlas cache. Normal launchers retain their previous behavior; user.ltx shadow settings are not changed.

Local shadowed lights are temporarily routed to the existing unshadowed spot/omni accumulation path. Their shadow flags are restored at function exit. Illumination is retained, while local caster-list construction, atlas packing and shadow depth drawing are bypassed.

Both legacy and newer sun paths retain projection setup, depth clears and sun accumulation, but skip caster geometry drawing. This leaves sun-light setup/culling CPU work in place; the test is not a measurement of a renderer with every shadow-related instruction removed.

SSAO, baked/static shading, cloud modulation and rain roof-occlusion handling remain unchanged. These can still produce dark regions and are not object-cast dynamic shadow maps. Flashlight and other local object-cast shadows use the unshadowed route too.

Native Game build and launcher PrepareOnly verification passed. No game launch or visual confirmation was performed by the agent. Repeat the Skadovsk/binocular route, close the game and report completion to compare the profiling data.

Published alpha assets remain unchanged.

## Isolated tests

- Launch-Test-No-Local-Shadows.cmd disables local-light/flashlight caster shadows, retains sun shadows.
- Launch-Test-No-Sun-Shadows.cmd disables sun caster drawing, retains local-light shadows.

Both keep MSAA 4x, the same eye size and profiling, with the shadow cache off. Flags do not persist to user.ltx. The combined No-Shadows launcher remains available. Compare the same save and a similar route; optical views must actually be active to compare binocular cost.

Capture 20261026-170458-935 confirmed a broad improvement without dynamic shadows (typical active intervals around 14 ms/frame and 6-8 ms GPU elapsed). No optics_render_cpu rows were recorded in that capture, so it cannot establish the binocular improvement. Routes are not frame-identical; no precise speedup percentage is claimed.
