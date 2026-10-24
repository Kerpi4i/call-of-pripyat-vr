# Inventory unload and physical props (alpha 1m)

Controls:
- Inventory: hold index trigger to drag a firearm, then press the same Touch grip
  to unload that exact weapon through native UnloadMagazine. No stacked children
  are unloaded, no physical extraction occurs, and inventory remains open.
- World: with an empty hand, bring the palm within 16 cm of a movable physics
  element and press/hold grip. Release grip to let go. Works on non-inventory
  dynamic shell objects, including crates and dead NPC/mutant ragdolls.
  Inventory loot retains its existing separate pickup behavior.

Implementation:
- Native main/detector/support hands, world loot and belt ammo reserve their hands.
- A bounded damped spring applies force at a selected element-local contact point.
  Existing physics, gravity, ragdoll joints, and world collisions remain active.
  No shell teleport, rigidification or world-wide collision disabling.
- Per actor/object contact suppression prevents camera shaking. Uses existing
  release contact guard, now generalized to PhysicsShellHolder.
- Grip state stores object ID and element index and resolves them each update.
  Release on invalid object, lost tracking, blocked gameplay, grip release,
  fixed element, excessive separation, or actor network teardown.
- Live creatures, inventory items, fixed elements and static level geometry are
  excluded. Crates represented only as static geometry cannot be lifted.
- Release leaves current physics velocity intact. Heavy-body feel, rotational
  response and force tuning remain experimental and require headset validation.

No game or OpenXR test was launched during implementation.

Validation: xrGame Release x64 build succeeded; engine diff whitespace and reverse patch checks passed. Gameplay behavior is not yet verified.
