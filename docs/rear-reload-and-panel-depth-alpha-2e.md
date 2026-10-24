# Rear reload chord and hand-panel occlusion, alpha 2e

Disable debug_zones. Hand-panel shader passes now enable depth testing without depth writes (zb true,false), so native world depth can occlude them instead of always drawing over the scene.

Remove ammo belt acquisition/hover loop and belt-mounted ammo proxy rendering. A held reload activator still uses the existing visual and release-at-weapon mechanism. The old ammo debug box is removed.

The shared rear waist zone keeps plain-grip knife/bolt selection. If a firearm is active in one hand, the opposite free hand can acquire its reload activator with grip + index trigger. Use the same 90ms chord grace as chest grenade selection so grip arriving first does not immediately select knife/bolt. Claim the chord before generic F input. The selected native ammo type and real inventory ammo counts remain authoritative.

After acquiring, grip holds the proxy; release near the original weapon triggers native reload, release elsewhere cancels. World/inventory ammunition is unchanged and does not trigger this gesture mechanism. No independent secondary weapon system.

Validation: final xrGame build succeeds after the user closed the test; existing body-zone and two-hand tests pass; diff/patch checks pass. Hand-panel occlusion and rear chord still require headset validation. No game launch by this task.
