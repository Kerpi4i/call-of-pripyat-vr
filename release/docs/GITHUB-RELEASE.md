# Call of Pripyat VR — v0.1.0-alpha.2

**Pre-release / Альфа. Original Steam Call of Pripyat 1.6.02 required.** Not Enhanced Edition or a standalone Quest application. Free unofficial PC VR mod, tested during development with Meta Quest 2 / Touch / USB Quest Link.

## Changes since alpha.1

- Fixed local-light shadow popping during headset pitch; user confirmed both-eye lighting remained correct.
- Separate 2080×2096 eye buffers improve clarity at the tested Link settings. Desktop mode remains separate. MSAA 4x is the default and is no longer forcibly disabled on launch.
- Same-frame local shadow reuse, frame-stable map sizes, GPU relocation of individual light maps and pipelined shadow preparation reduce repeated work between views. Tests show reduced long-frame tails in some scenes; no universal FPS gain is promised.
- Stronger adaptive smoothing for Touch-driven UI rays; UI panel geometry no longer follows the eye-buffer aspect ratio.
- Winchester/SPAS pump cycling uses the support-grip interaction, without the duplicate standalone bolt contact.
- Binocular imagery omits dynamic caster shadows, retaining illumination. Weapon scopes and the main world keep shadows. This change did not clearly improve performance in the latest test.
- Added opt-in development CPU/GPU profiling and comparison launchers. Diagnostics are off in the player launcher.

## Install

Download **CoPVR-0.1.0-alpha.2-Setup.exe** and its companion `.config`, or the portable ZIP. Check **SHA256SUMS-0.1.0-alpha.2.txt**. Install into a new separate empty folder; do not overwrite the Steam game or an existing mod install. This installer does not upgrade a nonempty alpha.1 folder in place. Keep the old install and saves until you have checked the new one. Open CoPVR.exe, select the original game, enter Quest Link and Play. English/Ukrainian offline guides and status tables are included.

## Known limits and evidence

The campaign has **not** been completed in VR. Cutscenes, scripts or quests may misbehave or block progress. Back up saves. Other headsets/GPUs, seated play and combinations with other mods are unverified. Heavy Skadovsk views and binocular use can still miss the 72 Hz budget and fall to about 36 application FPS or lower. MSAA costs GPU time; reduce it in settings if necessary.

Development headset testing is not a clean-install acceptance test of this new installer. Automated builds, resource checks, depth-copy tests and installer extraction/self-check are recorded separately in the validation asset. Alpha.1's historical installation acceptance must not be read as alpha.2 acceptance. Full regression of all weapons/interfaces remains incomplete.

Point-face culling, conservative sun-frustum experiments and global no-shadow modes are not enabled by the player launcher. D3D compiler absence falls back to ordinary shadow rendering rather than disabling illumination. Discarded magazines retain shared ammo-pool accounting, and unrestricted independent dual firearms remain unsupported.

## Українською

Оновлення виправляє зникнення локальних тіней при нахилі голови, покращує чіткість і додає MSAA 4x за замовчуванням та оптимізації повторного використання тіней. У біноклі динамічні тіні вимкнені, у прицілах і головному світі — збережені. Важкі сцени та бінокль досі можуть сильно знижувати FPS. Повного проходження кампанії немає; можливі проблеми катсцен і квестів. Інсталюйте в нову порожню папку, зберігши старий мод і сейви. Новий інсталятор має окрему автоматичну перевірку; це не замінює перевірку в гарнітурі після чистого встановлення.

Report version, headset/runtime, GPU/CPU, reproduction steps and logs. Review logs for personal paths before sharing. No game archives in issues.

Built on OpenXRay and Khronos OpenXR. Game content belongs to GSC Game World and its licensors. Not affiliated with GSC, Valve or Meta. See bundled licenses and notices.
