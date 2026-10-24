# Scope reticles and eye-box cue, alpha 2l

All seven detachable-scope aperture profiles move +0.010 m along model Z, into the sight body. Binocular calibration is unchanged.

Read scope_texture from the actually attached addon. PSO variants use native wpn/wpn_crosshair; SUSAT variants use native wpn/wpn_crosshair_l85. The dedicated scope pixel shader samples these textures using the same collimated angular UV as the scene. Alpha separation removes the baked translucent glass and an inner mask removes the original full-screen housing, retaining the stock scale/aiming marks. It does not add a new flat HUD overlay. Fixed magnification and stereo scene cameras remain unchanged. The scale artwork is native; no claim of newly calibrated ballistic range marks is made.

A scope-only directional vignette uses eye displacement across the optical axis and aperture coordinates. It is smooth, symmetric under mirrored eye displacement and bounded at 28 percent darkening. Binocular rendering remains unshaded. The existing out-of-eye-box black cutoff remains.

Validation: xrGame build succeeded; copvr_scope pixel shader compiled with FXC ps_5_0; binocular/zoom/proximity and directional-shade tests pass, including centred brightness, opposite-side attenuation, mirrored displacement and lower bound. Headset appearance requires verification. No game launch.
