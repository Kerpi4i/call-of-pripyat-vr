# Clear jams by charging stroke

The existing physical charging-handle gesture now clears a native misfire without requiring the magazine to be removed/replaced first. The handle still requires its existing full rear threshold (90% travel) and release; incomplete strokes never invoke VRRack. Native clearing animation and mechanical sound remain as before.

The pure magazine state accepts a jam-clearing rack even without usable ammo, while preserving its chamber-readiness flag in that case. Removing/inserting a magazine still requires the normal subsequent cocking stroke for weapons that use it. Pump/internal weapons retain their existing full-cycle clearing behavior.

Regression tests cover racking a jammed loaded magazine directly, clearing a jam without a magazine without bypassing readiness, and normal reload/empty/gauss behavior. Game not launched.
