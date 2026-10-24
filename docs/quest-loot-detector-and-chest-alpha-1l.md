# Quest loot, native detector compatibility and chest reach

User confirmed sleep, tracked shadows and physical crouch work after rollback.
Independent second-hand equipment remains disabled.

Changes:
- World grip no longer rejects quest items. Close grip requests native pickup
  after CanTakeItem; no physics shell is required for quest loot. Quest loot
  cannot be extracted or thrown from inventory. Original archive configuration
  confirms af_quest_b14_twisted (the wheel artifact) has quest_item=true.
- Local VR detector accepts all main HUD weapons and no longer auto-hides for
  reload/zoom/climb. This uses existing primary+detector HUD slots, not extra state.
- Chest detector/binocular sphere reduced from 25 to 14 cm radius; center moved
  from 16 to 7 cm forward, still 36 cm below estimated torso head anchor.
- Renderer rejects original current-view actor visual in VR at add_Visual,
  including transient light passes. Separate tracked visuals remain allowed.
  This is a defensive fix for the shot-time mesh flash; headset validation pending.

Validation: xrGame and xrRender_R4 built successfully; engine diff and reverse
patch application checks passed. Game was not launched. Grip quest pickup,
all-weapon detector combinations, reach and muzzle-flash mesh behavior still
require headset testing. No claim of visual confirmation.
