# SPAS open loading / RG6 pull latch and drum animation

SPAS:
- Profile load_requires_open=1 gates both proximity feedback and authoritative ammo consumption.
- Loading requires the pump/bolt at least 90% rearward with the rear latch reached. Partial or closed movement rejects the round without consuming inventory.
- A full physical charging-handle stroke leaves the bolt latched back; it no longer completes an automatic forward stroke. The foregrip/pump can be returned forward to close it. A subsequent completed direct handle gesture also releases it forward.
- Loading while rearward preserves the open state and cannot bypass the closing requirement. Winchester and Protecta keep their preceding behavior.

RG6:
- Native lock motion remains authored; the reload/front assembly uses the same relative authored hinge axis with an expanded 180-degree full-open rotation. The stored angle remains the existing normalized 0..0.65 progress for save compatibility.
- A fresh grip at the existing latch grabs it. Pull at least 4 cm toward the rear of the weapon to toggle open/closed exactly once. Pressing or releasing without pulling does nothing. Release and grab again for another operation.
- Each shot/insertion advances the drum, including the last shot and sixth insertion. Rotation is animated forward rather than snapping, at 1 revolution per second (about 0.17 sec per chamber). Skipped chambers take proportionally longer.
- The pump field of the existing drum state stores the normalized rendered drum phase. Shooting and insertion require the drum to finish indexing. Rendered drum and loaded VOG meshes share that phase. It is saved in the existing RVI1 payload.

Validation: state tests cover closed/partial SPAS rejection, fully rearward loading, closing readiness, all six drum inserts/shots, final advancement, animation intermediates and action blocking during rotation. Asset validator checks the SPAS open-only flag. Native rendering and interaction feel still need a headset test; game not launched.
