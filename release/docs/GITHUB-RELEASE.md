# Call of Pripyat VR — Alpha 0.1.0

**Pre-release. Unofficial free PC VR mod. Original Steam Call of Pripyat 1.6.02 required.** Not Enhanced Edition; not a standalone Quest app. Tested during development on Quest 2 / Touch / USB Quest Link. The campaign has NOT been played through from start to finish in VR. Cutscenes, scripted scenes and quests may fail or block progress. Keep backup saves. Other hardware is unverified.

Tracked headset/hands, hand-aimed weapons, support grips, physical magazine and single-round reloads, stereo binoculars/scopes, body-based equipment/quick slots, physical item handling, VR menu rays, wrist HUD and physical crouching are implemented. Historical headset confirmations and pending checks are distinguished in the included English/Ukrainian feature-status guides.

Download **Setup.exe** with its companion `.config` for guided installation, or the portable ZIP. Verify against SHA256SUMS. Setup uses a separate folder and the original game's archives; it does not replace Steam files or import private saves. Open `CoPVR.exe`, enter Quest Link, then Play. Read the included EN/UK offline guides before using reload/body gestures.

Graphics defaults are frozen from the current development configuration (2560×1440 plus its quality/distance values); lower them if needed. R4, borderless, MSAA off and HDAO off are compatibility constraints. A prior incompatible imported graphics combination caused a black-screen PC hang; the corrected launch was confirmed working, but broad GPU qualification has not been performed.

Important alpha limits: discarded magazines use shared ammo-pool accounting; unrestricted independent dual firearms are not supported; seated play and other mods are unverified; save-load after death requires explicit acceptance because of a historical crash; final near-plane/zone-toggle/latest reload fixes need their recorded regression results. The owner confirmed installation and launch working on 2026-10-25. This is installation/startup acceptance, not certification of every feature or a full campaign.

English: `docs/USER-GUIDE.en.html`, `docs/FEATURE-STATUS.en.html`.
Українською: `docs/USER-GUIDE.uk.html`, `docs/FEATURE-STATUS.uk.html`.

Report version, headset/runtime/GPU, clean-install status, steps and relevant logs; review usernames/local paths before sharing. No game archives in issues.

Built on OpenXRay and Khronos OpenXR. Game content belongs to GSC Game World and its licensors; see bundled notices. Not affiliated with GSC, Valve or Meta.

Installation/startup acceptance: owner confirmed working on 2026-10-25 and authorized publication. Native build, all 24 C++ tests, resource/dependency checks and installer extraction passed. Unverified feature checks remain listed in the guides.
