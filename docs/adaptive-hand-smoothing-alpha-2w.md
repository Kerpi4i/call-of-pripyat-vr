# Weak adaptive tracked-hand smoothing, alpha 2w

Add a speed-adaptive One-Euro-style pose filter for each controller in tracking-reference coordinates. Default position cutoff 12 Hz/beta 20, angular cutoff 10 Hz/beta 8. Filter derivatives at 1 Hz, with a small instantaneous-speed floor to reduce the first-frame fast-motion delay. At rest this reduces controller jitter; fast motion raises cutoff and follows more closely. Position lag is bounded to 15 mm and angular lag to 2.5 degrees. These are ceilings, not an imposed constant delay.

Smooth one physical grip pose, then apply its rigid delta to both aim and grip frames. Preserve the runtime's aim/grip relationship and authored palm offsets. Cache by input timestamp so repeated game/render consumers do not filter again in the same sample. Transform to world space only after filtering, preserving actor movement and snap-turn response. Do not alter headset tracking, bridge input samples, triggers/buttons, stereo rendering or independent UI pointer input.

Reset on inactive/lost/invalid tracking, >100 ms timestamp gaps, recenter-like position jumps >35 cm or orientation jumps >90 degrees. Handle quaternion sign equivalence and shortest-path interpolation. Native weapon aiming, hand IK, body interactions and visible held items use the consistent filtered pose.

Maintain a separate raw reconstruction path: invert the filtered grip delta to recover the corresponding unfiltered held pose. Use it for bolt/grenade/weapon/world-item/physics-object throw histories and physical knife sweeps and damage-speed estimation. These do not inherit the new display smoothing. Released objects still originate at their current displayed held pose. Very fast interaction/render alignment needs headset verification.

[Hands] settings in copvr_equipment.ltx (actual section name is lowercase hands): smoothing_enabled, position_min_cutoff, rotation_min_cutoff, position_smoothing_beta, rotation_smoothing_beta. Restart after edits. Set smoothing_enabled=false to restore direct controller following. Existing accuracy multipliers are unchanged.

Validation: adaptive pose tests cover jitter energy reduction, duplicate timestamp consistency, fast-motion lag caps, quaternion sign equivalence, teleport/gap reset and invalid-pose reacquisition. Knife, optics and scope state tests passed. No game launch; perceived latency/comfort and live throw/melee behavior remain unverified in headset.
Final xrGame build succeeded; adaptive filter, knife, scope and optics tests passed; engine patch checks passed.
