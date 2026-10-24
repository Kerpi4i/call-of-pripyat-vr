# Scope-use accuracy bonus, alpha 2r

Apply an additional 0.65 dispersion multiplier only during actual optical sight use. The bonus stacks multiplicatively with supported_dispersion_scale=0.55: together they retain 0.3575 of native bullet/pellet spread (64.25 percent reduction). Recoil, aim direction, ammunition characteristics and damage are unchanged. It applies once in the common firing path, alongside existing local VR singleplayer ownership guards. Both detachable and permanent optical sights qualify; binoculars do not.

Record scope visibility from the actual eye render: optical image ready, valid eye relief, gaze direction dot optical axis >0.95, and central ray through the scope tube within normalized radius 0.85. One valid eye is sufficient. Clear the observation once per render frame, not on the second eye; observations expire after 150 ms if rendering stops. Weapon identity is checked so switching weapons cannot transfer the bonus. This intentionally requires a rendered usable pupil rather than merely an installed scope or legacy zoom flag.

Configuration: [hands] scoped_dispersion_scale=0.65, clamped [0.1,1], independent from support-grip scale. A prior rendered frame is used for the next simulation/firing update, so acquisition/loss follows the normal render/input frame boundary.

Validation: scope-accuracy state tests cover unobserved state, weapon mismatch, second-eye frame preservation, new-frame reset, expiry and unsigned-clock wrap. Existing optics tests pass. No game launch; live shot grouping needs headset verification.
xrGame build succeeded; scope state and optics tests, engine patch checks passed.
