# Rollback of independent equipment (alpha 1k)

Independent second-hand equipment registration is disabled. Belt, rear head slots,
world weapon pickup and inventory grenade extraction all equip through the native
inventory/HUD path again, including when the other hand has an active item.
Detector compatibility and automatic visibility restrictions are restored.
The independent second-hand implementation is no longer reachable; compatibility
state/render hooks remain inert for a future replacement.

Preserved: tutorial sleep input routing and removal of its duplicated prompt,
weapon draw zone at/above head height, chest detector/binocular access, and tracked
hand/item shadow rendering instead of the standard actor model.
These preserved changes still need headset validation.

No game test was launched for this rollback.
