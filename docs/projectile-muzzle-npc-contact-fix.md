# Projectile muzzle / NPC contact correction

Projectile paths:
- Cache the final tracked HUD item transform after VR weapon pose and mechanical-part updates. Cache is keyed by actual weapon pointer and limited to two frames of age.
- RPG7, RG6 and rifle underbarrel launch paths compute the projectile origin from the native HUD muzzle bone/offset transformed by that tracked pose, after native g_fireParams has run.
- Underbarrels select the secondary muzzle, RPG/RG6 the primary muzzle. Direction uses the rendered weapon forward axis.
- RG6 and underbarrel VR paths bypass the native zoom/camera ballistic auto-aim correction. Native physics still determines the grenade trajectory after launch.
- The isolated snapshot avoids attachable_hud_item::setup_firedeps/update overwriting the tracked item transform with a standard actor-hand attachment before projectile creation.

NPC contacts:
- In VR, non-player CEntityAlive objects reject actor-camera collision. This is consulted by both the camera-shell callback and the early actor/object movement-contact filter, including skeletal/ragdoll shapes that are not tpCharacter.
- CPHActorCharacter::InitContact also rejects eligible AI character contacts before it can construct special restrictor/step joints, including the ph_object-only fallback when reference metadata is incomplete.
- Hit collision forms, bullets, melee rays, NPC/NPC contacts and static world geometry remain independent.

Validation: Release xrGame/xrPhysics build, existing weapon/trigger/reload regression tests. No game launched. Visual muzzle-origin alignment and walking directly through NPCs still require in-headset verification.
