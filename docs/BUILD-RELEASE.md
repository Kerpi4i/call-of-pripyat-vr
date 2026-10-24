# Building the alpha release

Version: `0.1.0-alpha.1`. This is a candidate; publication is gated by `release/docs/CLEAN-INSTALL-CHECKLIST.md`.

## Prerequisites

Windows x64; PowerShell; Git; Python 3; Visual Studio C++ build tools; .NET Framework C# compiler at `%WINDIR%\Microsoft.NET\Framework64\v4.0.30319\csc.exe`. For rendering the offline docs/icon: `python -m pip install Markdown Pillow`. Python dependencies are only for developers, not players.

Run from the repository root:

```powershell
powershell -ExecutionPolicy Bypass -File tools/bootstrap-portable.ps1
powershell -ExecutionPolicy Bypass -File tools/bootstrap-openxr.ps1
powershell -ExecutionPolicy Bypass -File tools/prepare-engine.ps1
python tools/render-release-docs.py
python tools/check-release-resources.py
powershell -ExecutionPolicy Bypass -File tools/build-release.ps1
```

`prepare-engine.ps1` pins OpenXRay to `c62d9d343981d50f6ed176412b85d94c61b578b5`, retrieves its submodules/SDK and applies `patches/openxray-vr.patch` without discarding local changes. Native builds use the repository's `src/` hooks. The bridge uses the bundled pinned OpenXR headers and locally downloaded loader. The Windows launcher/setup are compiled from the C# files in `release/launcher/`.

The tracked `release/gamedata/` overlay is the frozen alpha data, including native-engine config/script overrides and game-derived mod assets. No archives, saves or development installation are needed for packaging an already built engine. Installation/play still require the original game. `tools/snapshot-release-assets.py` is a **maintainer-only** refresh operation: it snapshots `game-vr/gamedata` and the current development graphics config, removes personal monitor identity and disables test kits. Do not run it on arbitrary local mod data for a public build. Review manifest changes.

`tools/build-release.ps1 -SkipNativeBuild` packages already verified native outputs. It refuses to overwrite an existing stage/version archive. Preserve or rename an older candidate deliberately before rebuilding. It does not launch setup or the game. Output: versioned setup EXE, portable ZIP, source ZIP, launcher runtime config and SHA256SUMS in `dist/`. The build adds a per-file integrity manifest inside the ZIP; setup verifies it before configuring paths.

The installer is a per-user, offline, .NET WinForms self-extractor with an embedded ZIP, a game-folder selector and optional desktop shortcut. Native game binaries are in `bin/`; original archives are referenced through generated `fsgame.ltx`. Settings/saves are in `data/`. No elevation, game-directory modification, driver install, runtime-registry changes, automatic game launch, download or publication occurs.

The included unsigned EXEs have no Authenticode certificate; SHA256SUMS detects download mismatches but does not establish publisher identity. Sign final binaries if a suitable certificate is available, then regenerate checksums. No certificate is invented or included.

## Validation

- Compile/run all 24 `tests/*_tests.cpp` with Zig C++17, `-Isrc -Ithird_party/openxr/include`; bridge ABI test additionally needs the built Windows bridge.
- Run the four `tools/validate-*.py` validators in the development tree for magazine/internal/underbarrel/audio assets. Their archive-inspection inputs are developer diagnostics and are not part of the end-user ZIP.
- Run `python tools/check-release-resources.py` on any source checkout.
- Verify packaged DLL import closure and SHA256; smoke-test `CoPVR.exe --self-check` (no game/UI/runtime creation).
- Setup supports internal `--verify-extract <new-empty-path> <original-game-path>` to exercise the same extraction/path generation **without** registry/shortcut/UI/game launch. This is a developer integration test, not final installation acceptance.
- The interactive installer, shortcuts, uninstaller, launcher UI, OpenXR and new-game/death-load paths must still pass the human clean-install checklist before publishing.

## GitHub publication preparation

No repository has been created or pushed by this task. Choose the repository owner/name, add the remote, commit reviewed source, then tag `v0.1.0-alpha.1` only after acceptance. Do not add `engine/`, `game-vr/`, `reports/`, `build/`, private saves or game archives. They are ignored. Source distribution consists of this repo, a pinned engine-bootstrap script/patch, frozen overlay, attribution and build instructions.

Use `release/docs/GITHUB-RELEASE.md` as the **pre-release** body; attach the setup EXE, its `.config`, portable ZIP and SHA256SUMS. Mark the release as a prerelease, not a stable release. Run `.github/workflows/source-checks.yml` for portable checks; this workflow does not claim to build/test the native VR package. No automatic publication workflow is configured. Put the actual GitHub release URL into the Steam post drafts immediately before posting. Do not post a placeholder URL.
