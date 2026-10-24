# Support accuracy and fused binocular pupil, alpha 2o

## Supported weapon accuracy

At the common CWeapon firing path, multiply final bullet/pellet dispersion by 0.55 while a genuine support grip is active. This is a 45 percent reduction in angular spread, including shotgun pellet cones, without changing ammunition damage, recoil or controller aim. Apply once before the pellet loop. Eligibility requires VR singleplayer, current view/control owner, matching weapon grip profile, locked enabled support state, live tracking of both hands and a held support squeeze at or above the existing 0.25 release threshold. This works for either primary hand. AI, desktop, unsupported/one-handed weapons and lost tracking retain their native spread. Native ammunition, condition and modifier effects remain in the incoming dispersion value. [hands] supported_dispersion_scale = 0.55 is restart-configurable and clamped to [0.1,1].

## Common binocular tunnel mask

Keep both original collimated stereo scene cameras and their sample coordinates. Use the tracked head centre relative to the binocular axis to compute one common angular elliptical pupil. Both eyes use the same mask function and head reference; equal angular image coordinates receive equal mask values. Do not ray-test separate left/right tubes or move/converge the scene images. Head displacement shifts the common pupil, producing an opaque black crescent and a softened inner edge. The mask size follows shared eye relief and the existing proximity aperture size. Both binocular lenses use the existing 16-bit mask-coordinate decoder used by the scope shader.

The standard physical aperture boundaries and near-eye safety guards remain; this does not claim zero binocular rivalry at every extreme pose. Headset assessment of fusion, mask size and offset sensitivity is pending. Rifle scope tube/reticle rendering is unchanged.

Validation: binocular pixel shader compiled with FXC SM5, shared-mask centre/shift/edge tests and previous optics tests pass. Reviewed the common firing path and weapon identity check for either-hand supported fire. No game launch or empirical weapon grouping test.
Final xrGame build succeeded; optics tests and engine patch whitespace/reverse-application checks passed.
