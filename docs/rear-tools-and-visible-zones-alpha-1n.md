# Rear tools and visible activation zones (alpha 1n)

- Empty left hand + fresh grip in rear waist zone selects native knife slot;
  empty right hand + grip in the same zone selects native bolt slot.
  Torso-relative center (0,-.61,-.34), half extents (.42,.14,.16) metres.
- Knife and bolt have no belt model or belt grab target. Existing hold/release
  behavior is retained; no independent second-hand item state is enabled.
- Chest draw and inventory stow now share the exact same sphere in the torso
  frame: center (0,-.36,.02), radius .11m (previously .14 at z=.07).
  Empty grip: left detector/right binocular. Release pickable loot: inventory.
  Old broad headset-relative inventory volume is removed.
- Quick pocket anchors move 5-6cm closer to torso; ammo moves 4cm closer.
  Grenade moved to side at z=0 to avoid overlap with the new rear tools volume.
- debug_zones=true in copvr_equipment.ltx enables world-space wire volumes:
  cyan chest/stow; orange rear tools; purple weapon draw and pale purple return
  extension; yellow temple flashlight; green ammo and current reload port;
  peach quick slots/grenade. Belt zones appear when corresponding item exists.
- Dedicated unlit HLSL shader renders alpha wires without depth rejection for
  debugging. RenderUI draws them before flat UI capture, separately for each eye.
  Shared helpers define interaction coordinates and contour coordinates.

Validation: new chest/back boundary tests passed (old large stow area excluded,
front/weapon rack separation, symmetric reach, invalid coordinates). VS/PS compiled
with fxc shader model 5.0. Headset visibility/reach remains unverified.
No game test launched.

Final validation: xrGame Release x64 build succeeded; engine diff and reverse patch checks passed.
