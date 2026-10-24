# VR reload motion suppression, alpha 3a

For actor-owned weapons under -copvr, CWeaponMagazined::switch2_Reload stops the old animation callback, performs native ReloadMagazine accounting immediately, clears pending and requests idle without calling PlayAnimReload. This bypasses full/empty/misfire variants, revolver, double barrel, RPG and attached-launcher rifle reload clips. The underbarrel grenade branch independently uses the same no-motion completion and preserves grenade rocket ownership and reload sound.

Both CWeaponShotgun and CWeaponAutomaticShotgun route VR reload requests, state switches and completion callbacks through the base path instead of the open/insert/close tri-state animation loop. Flat mode and NPC behavior retain native animations. Reload depth-of-field camera effector is disabled in VR. Existing physical charging-handle gestures currently request the same native reload action, so they also cannot start canned reload/cocking clips.

This checkpoint only suppresses canned reload motions; it does not yet implement the requested detachable magazine extraction, discarded magazine, insertion and chamber-required firing state machine. Native ReloadMagazine still performs ammunition transfer immediately. No claims of physical magazine implementation are made here.

Validation: xrGame Release x64 build succeeded; engine diff whitespace and reverse patch application checks passed. Source audit covered all native PlayAnimReload overrides, both tri-state shotgun implementations, and the grenade reload override. No game launch or headset test performed.
