# Call of Pripyat VR

Unofficial free **PC VR alpha** for the original Steam **S.T.A.L.K.E.R.: Call of Pripyat 1.6.02**, built on OpenXRay and OpenXR. Requires the original purchased game. Not Enhanced Edition; not a standalone Quest APK. No game archives are included.

**Alpha: 0.1.0-alpha.1. Installation and launch confirmed by the owner on 2026-10-25. Individual feature regression remains incomplete.** Development headset: Meta Quest 2 / Touch / USB Quest Link. A full campaign, other hardware and all weapon variants are not certified.

## Players / Гравцям

- [English user guide](release/docs/USER-GUIDE.en.md) · [Українська інструкція](release/docs/USER-GUIDE.uk.md)
- [Feature status / evidence](release/docs/FEATURE-STATUS.en.md) · [Стан функцій / підтвердження](release/docs/FEATURE-STATUS.uk.md)
- [Clean-install release gate / Перевірка перед релізом](release/docs/CLEAN-INSTALL-CHECKLIST.md)

Download the versioned Setup EXE + companion config or portable ZIP from [GitHub Releases](https://github.com/Kerpi4i/call-of-pripyat-vr/releases/tag/v0.1.0-alpha.1). Source ZIP is not the playable binary package. Install separately, enter Quest Link, open `CoPVR.exe`, choose the original game folder if needed, then Play. English/Ukrainian offline HTML guides are included. No development tools needed to play.

Keep the alpha label and known-issue documentation when distributing. A working install/startup is not certification of every interaction, weapon or campaign scene.

## Implemented

Stereo world and head tracking; tracked hands; controller-aimed firearms; support grips; manual magazines and single-round/internal reloads; physical knife; stereo binoculars/scopes; body equipment/quick slots; grabbing and throwing; menu rays; classic VR UI; wrist map/status; physical crouching. “Implemented” is not a claim of full final-build headset validation. See the status tables.

## Developers

[Build/package instructions](docs/BUILD-RELEASE.md), [contribution guide](CONTRIBUTING.md), [attribution/licenses](THIRD-PARTY-NOTICES.md). Source engine is pinned and supplied through a bootstrap script plus `patches/openxray-vr.patch`; the full engine checkout and original game are not committed. Frozen release resources are in `release/gamedata`.

Working developer launch remains `Launch-Game-VR.cmd`; release players use `CoPVR.exe`. Graphics defaults for this candidate are taken from the current developer config, with mandatory DX11/borderless/MSAA-off/HDAO-off compatibility settings. Saves and settings are isolated from the original installation.

Not affiliated with or endorsed by GSC Game World, Valve or Meta. All game content remains the property of its respective owners. Original CoPVR code uses the scoped MIT license; game-derived/upstream content retains its own terms.
