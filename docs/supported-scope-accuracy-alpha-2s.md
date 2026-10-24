# Scope bonus requires support grip, alpha 2s

Move the scope multiplier inside the same eligibility block that applies the two-hand support bonus. A supported, tracked, locked grip on this weapon and an aligned visible scope pupil must now both be present on the shot. One-handed scoped fire retains native dispersion. Two-hand fire without looking through optics retains the existing 0.55 multiplier. Two-hand scoped fire uses 0.55 * 0.40 = 0.22 (78 percent spread reduction). Releasing support immediately bypasses both bonuses regardless of cached scope visibility.

The fallback optical multiplier is aligned with the current configured 0.40. Existing native ammo/condition dispersion, AI and desktop behavior are preserved. Validation: reviewed the single firing/pellet-loop application and shared support predicate; existing scope-observation tests passed. No game launch.
xrGame build succeeded; scope state tests and engine patch checks passed.
