# Underbarrel VR alpha

Controls:
- Hold the ordinary foregrip with lower grip. A fresh upper-trigger press on the supporting hand toggles rifle/underbarrel mode. Trigger gates require release after focus/tracking/UI changes.
- In launcher mode, rear-waist grip+trigger supplies one matching native grenade proxy. Release at the green muzzle contact (7.5 cm radius) to insert exactly one owned compatible cartridge.
- The final 35% of the native anm_reload_g motion is played, with a matching final fragment of native reload audio. Pending state ends through the native idle callback. No full reload callback replenishes ammo.
- Repeating the supporting-hand trigger returns to rifle mode. Holstering or dropping also returns to rifle mode, preserving any loaded launcher round in its separate native magazine.
- Right stick down cycles native available fire modes while in rifle mode. It does not add unsupported burst/auto modes to a weapon. UI scrolling remains unchanged.

Coverage: AK74, upgraded Abakan, LR300, Groza, FN2000, SIG550, including 10 applicable configured variants. Each profile uses native HUD fire_point2/fire_bone2 for the muzzle, a native separate grenade mesh and authored grenade reload hand contact. Compatible grenade types remain ammo_vog-25 and ammo_m209.

Ammo safeguards: physical loading checks owner, compatible type, empty launcher, idle and pending state before consuming one cartridge. Native rocket ownership is created after insertion. A held proxy records whether it belongs to launcher mode and is cancelled on weapon or mode changes; it cannot become a rifle magazine. Normal rifle magazine state is not altered by GL switching.

Test copies in game-vr/appdata/savedgames:
- copvr_underbarrel_test: 10 launcher-capable weapon variants, addon flags attached, native Abakan launcher upgrade applied. Shared ammo reserves sum 100 grenades per compatible weapon.
- copvr_jam_test: all weapons from the existing all-weapons kit, 5% condition, shared reserves sum 100 rounds per compatible weapon and ammo type. Jam probability remains native, not forced.

These are copies of the latest ordinary save. Gear is provisioned once after loading each specifically named copy. Save in-game afterward to persist the issued inventory. Reopening the named copies replenishes missing kit/ammo; jam-test condition is applied on entry. Unlimited weight is restricted to the named test modes. Original save hashes are recorded and verified in reports/weapon-test-save-copies.json.

Validation: Release xrGame build, six launcher asset/audio profiles including original binary Vorbis metadata/CRC, original save hashes, all 10 kit variants and two grenade calibres, existing trigger/internal reload/magazine tests. In-headset mode switching, muzzle alignment and motion-tail appearance remain unverified. Game was not launched.
