# Binocular optics prototype, alpha 2g

The VR binocular now has two circular ocular surfaces showing a separate magnified scene. Bring the equipped binocular toward the eyes and point it along the viewing direction. Moving an eye away from an ocular darkens that lens. The surrounding stereo world keeps its ordinary field of view. No activation button or global headset zoom is used.

This prototype uses one shared monoscopic optical camera for both oculars, fixed at 15 degrees vertical field of view. It is not yet independent stereo optics or adjustable magnification. The optical camera follows the binocular model, not the headset. Rendering is gated by active equipment, solved tracked hand pose, proximity and alignment with the head; menus and dialogue disable it.

Rear ocular locations were measured from the original HUD model: local center (0.000073, 0.028223, -0.090084) metres, spacing 0.0953 m, radius 0.018 m. Configuration is in game-vr/gamedata/configs/copvr_equipment.ltx [binocular_vr]. optics_position/optics_rotation allow later model alignment correction. Actual alignment and eye comfort still require headset verification.

Implementation adds an optional game-to-engine optics camera query and a DX11 render target. Before the two headset eyes, the engine renders an additional full-resolution world scene and copies it into the optical texture. HUD hands, binocular surfaces and UI are excluded from that pass. Lens surfaces use physical depth testing, a circular mask and edge darkening. The extra full-resolution scene can substantially reduce performance while viewing; lower-resolution optics and independent per-eye cameras are future work.

Validation: Game target build succeeded, including xrEngine, xrGame and xrRender_R4. Binocular proximity/eye-box helper tests passed, including invalid pose rejection. The pixel shader compiled with FXC ps_5_0. Engine patch whitespace and reverse-application checks passed. No game or OpenXR session was launched for this change. Visual correctness has not yet been confirmed in a headset.

## Ocular-end correction

Headset feedback showed the optical surfaces were on the wrong end. Move them to local +Z (0.090084 m), reverse optical forward and horizontal axes together to keep a proper rotation and correct image handedness, and use a 0.028 m aperture on that end. Center is (-0.000074, 0.028715, 0.090084). This supersedes the initial rear-end assumption above. Eye-box and camera origin now follow the reversed optical basis as well. Added direction regression checks for an eye on local +Z looking toward -Z. Visual alignment remains pending headset verification. No game launch.
