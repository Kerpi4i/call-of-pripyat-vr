# Early held-object actor collision rejection and stronger throws (alpha 1r)

- Physics contact processing rejects VR actor/held-object contacts before
  material callbacks and PHObject::InitContact. The latter may create custom
  character movement/step contacts, so setting collide=false later in an object
  callback was insufficient. Uses the existing per-object actor-camera opt-out
  registry; all ragdoll elements reference the same protected shell holder.
- World geometry/other dynamic object contacts remain active for physics grips.
  Existing release guard keeps body/camera collision suppressed until separation.
  Flat gameplay retains its normal contact path.
- MotionHistory throw gain 1.25 -> 1.7; output speed cap 12 -> 24m/s, tracked
  velocity cap 12 -> 18m/s. Discontinuity rejection and angular limit retained.
- Physics grips now keep hand motion history and apply the release velocity
  impulse to all movable elements with their own mass. This gives crates and
  ragdolls the stronger throw path as well as ordinary inventory loot/bolts/
  grenades. Relative ragdoll element velocity is preserved by adding a common
  velocity change. No throw impulse on tracking loss, UI interruption or reset,
  or if the other hand still grips the same object.

No gameplay test launched. Native release behavior, shake reduction and throw
feel still require headset validation.

Validation: Game Release x64 build succeeded; engine diff and reverse-patch checks passed.
