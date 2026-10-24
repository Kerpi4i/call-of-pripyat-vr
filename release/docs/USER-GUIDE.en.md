# Call of Pripyat VR — Alpha 0.1.0

Unofficial, free PC VR modification for the **original Steam S.T.A.L.K.E.R.: Call of Pripyat 1.6.02**. Built on OpenXRay. Requires the purchased original game; no game archives are supplied. The owner confirmed installation and launch on 2026-10-25; individual feature regression remains incomplete. It is not affiliated with or endorsed by GSC Game World, Valve or Meta.

## Requirements and scope

- Windows 10/11 x64, .NET Framework 4.8 for setup/launcher, a DX11-capable GPU and a working 64-bit OpenXR runtime. No Python, Visual Studio or PowerShell configuration is needed to play through the EXE launcher.
- The tested setup is **Meta Quest 2 + Touch controllers + USB Quest Link**. Other headsets, Air Link, Steam Link, Virtual Desktop, SteamVR runtime and seated play are unverified. Bare-hand tracking is not implemented.
- Original/classic Steam game only. Enhanced Edition, Anomaly, other overhaul mods, multiplayer and cross-mod compatibility are not supported by this alpha. A full campaign playthrough has not been completed.
- There is no measured minimum GPU or guaranteed frame-rate target yet. The development machine uses an RX 6900 XT; this is not a minimum specification.

## Install and start

1. Install the original Steam game and launch it normally once to confirm it works. Close it.
2. Open Meta Quest Link on the PC. Set it as the active OpenXR runtime, connect the headset over USB and enter Quest Link.
3. Run `CoPVR-0.1.0-alpha.1-Setup.exe`. Confirm the detected original game folder or choose it manually. Choose a **separate, empty mod folder**, outside the Steam game directory. ASCII/Latin characters are required in both paths for this alpha. Installation needs no administrator rights in a user-writable folder.
4. Open the desktop shortcut or `CoPVR.exe`. Click **Play in VR**. Setup does not start the game automatically.
5. Stand upright, face your intended forward direction and **click the right stick** in gameplay. This recalibrates the room origin, forward heading, eye height and standing collision height. Calibration is blocked when crouching. Release triggers/sticks before entering gameplay.
6. Start a new game for the acceptance test. Existing saves can be copied manually into `data/savedgames`, but back them up first. Do not overwrite your original saves with VR saves; new weapon-state data is not guaranteed to be readable by the original executable.

The portable ZIP is an alternative: extract into a new user-writable folder, open `CoPVR.exe`, choose **Change game folder**, then Play. It does not register an uninstaller or create a shortcut.

All game archives remain in the original installation. The mod's `data/` directory contains its settings, saves, logs, screenshots and shader cache. Normal Steam launch remains separate. To uninstall a setup installation, use Windows Installed Apps or run `Uninstall.ps1` after closing the launcher/game; `data/` is preserved. With the portable edition, keep `data/` and remove the extracted mod folder manually.

## Controller terminology

**Grip** means the side/lower middle-finger trigger. **Trigger** means the front/upper index-finger trigger, sometimes called the bumper in development notes. Every grab uses a fresh grip press; passing through a zone while already holding something must not replace it. Interactions are measured at the virtual palm.

| Input | Action |
|---|---|
| Left stick | Head-relative movement |
| Left stick click | Sprint; unavailable during physical crouch |
| Right stick left/right | 30° snap turn |
| Right stick down | Cycle the equipped firearm's available fire modes; not holster |
| Right stick up/down while looking through binoculars | Adjust binocular magnification |
| Right stick up/down in menus | Scroll |
| Right stick click | Standing calibration |
| A | Jump |
| X | Inventory |
| B | PDA |
| Y / left menu button | Back / main menu |
| Grip | Grab, hold or take equipment from a body zone |
| Trigger with a firearm | Fire |
| Trigger with an item | Activate/use that item where supported |
| Trigger with an empty hand | Point at a target with that hand and use: dialogue, doors, switches, sleep and other native F actions |

Keyboard and mouse game input is disabled in VR. Use the Touch ray and trigger in menus; do not rely on a keyboard shortcut to recover a stuck menu. Windows controls remain available outside the game. Some native text-entry/edge-case menus are not covered by headset acceptance yet.

## Body zones — use without looking down

The full character body and floating item belt are intentionally hidden. The zones use an estimated torso frame with constrained head-following and movement alignment; physical torso/hip tracking is not available.

| Where to reach | What to do |
|---|---|
| Behind/around the head | Fresh grip takes the weapon from the corresponding native inventory weapon slot. Left hand selects the left slot; right hand the right slot. Returning/releasing a weapon in the head holster region stows it instantly with sound. Slot weapon type is not restricted to pistol/rifle. |
| Beside either temple, very close to headset | Short grip then release toggles flashlight. Hold grip about 0.65 seconds for night vision if your equipment supports it. A long hold does not also toggle the flashlight. |
| Upper chest | Left-hand grip takes the detector; right-hand grip takes binoculars. Release equipment to return it. Native simultaneous-equipment restrictions remain except explicitly relaxed detector/bolt combinations. |
| Upper chest, grip + index trigger | Take a primed grenade. Throw by moving your hand and releasing grip. Releasing inside the chest zone cancels priming and returns it. Do not use this chord accidentally. |
| Two adjacent zones below the chest | Left hand: left zone = quick slot 1, right = slot 2. Right hand: left zone = slot 3, right = slot 4. Assign their contents in the inventory. |
| Behind the waist | Left-hand grip normally selects knife; right-hand grip selects bolt. With a firearm equipped, the free rear hand can supply an auxiliary bolt. Knife extraction is inhibited in the conflicting right-firearm case. |
| Behind the waist, free hand, grip + index trigger | Take the appropriate reload object for the firearm in the other hand: magazine, shell, rocket or launcher grenade. This is a reload proxy, not a loose inventory ammo box. |
| Upper chest stow region | Release an eligible held world/inventory item to store it in the inventory. This shares the chest equipment region; it is not a separate invisible pickup pocket. |

To find zones, use **Settings → Gameplay → VR: interaction zones**, then Apply. The choice is saved. Outlines are off by default. The outline shows the zone volume, not a solid object. Some equipment/physical interactions have priority over overlapping gestures; first release both triggers and try with an empty hand if nothing is taken.

## Hands, items, inventory and PDA

- Hold grip to keep an item; release to drop/throw it unless a stow/return rule applies. Knife, detector and binocular equipment return rather than becoming loose world items. Bolts are thrown, and a replacement becomes available after throwing.
- A weapon picked up from the world becomes equipped when grabbed at its grip. Weapons can be transferred between hands. A real support-grip contact stabilizes two-hand aim and improves accuracy.
- World items can be picked up, inspected, passed and thrown. Crates, bodies and other selected physics objects have experimental grabbing. Held objects ignore player collision to avoid camera shaking. Scripted/quest objects are not guaranteed to behave like ordinary pickups.
- Pull an inventory item out with ray + grip. Use trigger for normal UI selection/dragging. To unload a firearm, start dragging it in inventory, looting or trade and press grip. The most recent implementations in all three windows require regression testing.
- Change ammunition type **in the inventory**, by dragging compatible ammunition onto the weapon. A loose ammo box taken from inventory/world cannot act as the body reload proxy.
- Activate supported consumables with the trigger in the holding hand. They no longer require moving to the mouth or a separate use zone. If released outside stow they can be dropped.
- Inventory, looting, trade, dialogue and PDA retain their classic layouts inside VR. Game overlays retain the stereoscopic world behind them. Main menu/loading use a panel. Text-entry, complex PDA pages and every trade workflow have not all been retested.
- Right hand has a minimap; left hand has native health/status information. Wrist panels are an alpha implementation. The standard flat HUD, quickbar and doubled interaction hints are suppressed.

## Firearms and manual reload

Automatic reload on an empty magazine is disabled. Native reload/draw/holster/cocking animations are suppressed where replaced by tracked interaction; selected jam and final insertion animations intentionally remain. Original sounds or cropped mechanical sound segments are used.

### Detachable magazines (including pistol magazines, PKM box and Gauss battery)

1. Grip the installed magazine with the free hand near its socket; remove it and release to discard.
2. Reach behind the waist with the free hand and hold **grip + trigger** for a fresh compatible magazine.
3. Move it into the weapon's magazine socket. It seats on contact while still held. Releasing an unused fresh magazine elsewhere returns the proxy.
4. Grip the slide/charging handle, pull fully back and release to chamber. Gauss does not require this manual chambering step.
5. A jam is cleared by a complete charging-handle cycle; changing the magazine is not mandatory. A partial pull is insufficient.

**Important limitation:** discarded magazines are temporary visual props, not persistent ammunition containers. Remaining ammunition is returned to the shared ammunition pool on removal; fresh magazines consume loose reserve rounds when installed. This is not a tactical magazine-retention simulation. Magazine hiding on dropped/world weapons and every weapon variant still require validation.

### Internal magazines and single-round weapons

| Weapon | Reload interaction |
|---|---|
| Winchester 1300 / Chaser-13 | Take individual shells behind the waist; insert one at a time at the loading port. Cycle the pump through its support grip after each shot. Partial topping-up is supported. |
| SPAS-12 | Pull the action fully open before inserting shells at the loading port. Close the action to fire; the prototype requires pumping after shots. This is the mod's interaction design, not a claim about real SPAS operating modes. |
| BM-16 / TOZ-34 | Grip the separate break-action contact forward of the ordinary support grip; open the barrels, eject spent shells, insert one shell per chamber, and close by gesture/handling the action. Ordinary support grip alone must not break the gun open. |
| Protecta / Eliminator | Insert shells individually at the receiver loading window. Manual cocking is required after loading from empty, not for topping up a nonempty gun. |
| RPG-7 | Take a full-size rocket and release it at the muzzle reload zone. The final native insertion animation/sound completes loading. No separate chambering gesture. |
| RG-6 / Bulldog-6 | Grip its latch contact and pull rearward roughly 4 cm to open. Load individual VOG rounds into the open drum; indexing is animated. Re-grip/pull the latch to close. The opening arc is 180°. Exact hand alignment and every firing/loading sequence need regression testing. |

### Underbarrel launchers

Hold the support grip and press that hand's index trigger to toggle grenade-launcher mode. Obtain a matching grenade from the rear-waist reload chord, then release at the underbarrel muzzle zone. A final native loading animation is intentional. Repeat the support-trigger toggle or holster to return to rifle mode. Physical muzzle origins were recently corrected and are not yet confirmed in-headset for every launcher.

## Knife, optics and movement

- Knife attacks use physical swept blade movement and speed-dependent damage, not the original button attack. Player/NPC collision was disabled for melee access, but the latest contact-path correction is not yet headset-confirmed. Bullet/knife hit detection remains enabled.
- Binoculars and detachable/integrated scopes use additional rendered views, reticles and a tunnel mask. Binocular zoom uses right-stick up/down. Support grip improves firearm accuracy; the additional optics bonus requires support grip and an aligned viewing eye.
- Weak adaptive hand smoothing is enabled; throws and knife speed use raw motion history. This is controller tracking, not optical tracking of bare fingers.
- Physical crouching changes the player collision volume. Crouching halves movement speed and disables sprint; the latest speed/calibration changes need acceptance. There is no crouch-toggle requirement for standing play.
- Sound orientation follows the headset. Camera motion in the wounded-NPC help sequence and sleep interaction were previously confirmed fixed; broader scripted camera coverage is incomplete.

## Graphics and stability

Release graphics are snapshotted from the developer's **current** working configuration, including 2560×1440 render dimensions and its current quality/distance values. Monitor name and refresh-rate identity are removed for portability. This may be demanding; reduce settings in-game if necessary. Some settings need a full restart.

DX11/R4, borderless window, MSAA off and HDAO off/HBAO selected are compatibility constraints enforced before launching. Other quality settings persist. Do not manually re-enable exclusive fullscreen/MSAA/HDAO for this alpha: a prior imported combination caused a black-screen system hang, and its precise cause was not isolated. The corrected launch was confirmed to work.

The latest 5 cm near-plane change targets world-surface depth flicker. Its headset result is **unverified**. Very close geometry may clip; lens protection has been adapted. Foliage shimmer may remain.

## What has actually been tested?

See [FEATURE-STATUS.en.md](FEATURE-STATUS.en.md). “Confirmed” means the tester reported the feature working at an earlier checkpoint on Quest 2. It does not mean every weapon, save, game location or the final installer has passed. Installer extraction, code builds and unit tests cannot certify headset comfort or campaign completion.

Known high-priority checks: clean install, save/load after death (a historical crash was not explicitly confirmed resolved), all newly changed launcher/reload/calibration paths, interaction-zone toggle, latest depth-flicker correction, every weapon variant and long sessions.

## Troubleshooting and bug reports

- No VR: confirm USB Quest Link is entered and Meta's 64-bit OpenXR runtime is active. The launcher checks the runtime registration, not whether the headset is actually ready.
- No movement/grabs after menu or lost tracking: release both grips/triggers, center sticks, regain tracking/focus, then try again. Do not recalibrate while crouching.
- Wrong zone alignment: stand upright and calibrate. Enable zone outlines in Gameplay. Body position is estimated from headset/controllers, so unusual lean/stance may still cause errors.
- Missing graphics/runtime DLL: reinstall the complete package rather than copying only `CoPVR.exe`. Windows/.NET requirements still apply.
- Before reporting, copy `data/logs/`, state the mod version, headset/runtime/GPU, save used and reproducible steps. Attach screenshots/video if useful. Review logs for your Windows username/local paths before sharing publicly. Do not upload original game archives.
- For a black screen/system hang, stop further tests and report what happened. The launcher cannot prevent or recover every GPU/driver hang.

There is no automatic upload, update service or telemetry in the launcher. No publication is performed by setup.
