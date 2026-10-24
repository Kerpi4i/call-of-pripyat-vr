# Alpha candidate validation — 2026-10-25

Candidate: `0.1.0-alpha.1`. Final package hashes are in `dist/SHA256SUMS.txt`. No public release, game launch or real headset session was performed by the packaging task.

Passed:

- Final x64 Release engine and OpenXR bridge compilation; bridge ABI checks.
- All 24 C++ test executables, including pose filters, stereo math, UI rays, body estimation/posture, hand symmetry, navigation, throwing/interaction, knife, support accuracy, magazine/internal reload, optics and frame modes.
- Asset validation: 26 magazine profiles, 10 lossless pistol splits, 33 part contacts, seven internal reload profiles, six underbarrel profiles, 57 mechanical audio excerpts with Ogg CRC/metadata checks.
- 859 frozen resource hashes and option-menu XML; both offline HTML/Markdown guides; test provisioning/weight boosts disabled.
- Final setup EXE verification extraction into a fresh temporary folder. The launcher completeness/self-check exited 0. Original Steam `fsgame.ltx` and `user.ltx` hashes remained identical.
- An attempted extraction into the now-nonempty test folder was rejected without replacing its launcher.
- Binary/source ZIP CRC checks; no game saves, PDB files or backup files included.
- Required PE import closure passes. `xrRender_GL.dll` is included because the executable imports it during bootstrap, although VR selects R4. Ansel is an optional delay-loaded dependency and is intentionally omitted. Discord integration is compiled out.
- Launcher UI visually inspected in Ukrainian and English at the current Windows DPI; scaling/text clipping corrected. Setup UI and Steam auto-detection visually inspected. The Install and Play buttons were not activated during that UI inspection.

Not yet passed:

- Actual interactive installation into the intended permanent folder, shortcut creation/Installed Apps registration and uninstall/reinstall acceptance.
- Launch through the installed launcher with Quest Link; new game/save/load/death-load regression.
- Final headset validation of zone toggle, latest depth-precision change, optics proximity, reload refinements, auxiliary bolt, full calibration, crouch speed and shadow/NPC-contact fixes.
- Campaign completion, other headsets/runtimes/GPUs and long-session performance qualification.
- GitHub repository creation, tag, upload and Steam publication. These remain unperformed pending owner acceptance.

Native binaries retain ordinary compiler build-path strings in assertion/debug metadata. No private saves, personal gameplay logs or user settings beyond the sanitized graphics snapshot are packaged. Source paths in generated installation files are produced locally by setup and are not taken from the development machine.

The clean-install checklist is intentionally left for explicit tester acceptance rather than silently marking headset checks complete. Technical extraction tests do not replace that checklist.

## Owner acceptance — 2026-10-25

The owner reported installation and launch working and explicitly authorized publication. This is a user-reported acceptance; it does not silently mark every per-feature/headset regression item above as passed. Publication URL/status will be recorded after remote verification.
