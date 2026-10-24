# Graphics configuration

Normal run-game-vr.ps1 / Launch-Game-VR.cmd launches preserve the VR user.ltx. The first migration imports Steam _appdata_/user.ltx byte-for-byte with a backup of the preceding VR configuration. Original Steam settings are never written by the VR launcher.

The original configuration was imported for this update. Further VR menu changes are saved to game-vr/appdata/user.ltx and remain intact on normal launches. To import Steam settings again, use -ImportOriginalConfig; -OriginalConfig can select another source path. -PrepareOnly never launches the game.

All quality/distance/effect/resolution/window/VSync rewrites have been removed from the default launcher. -HighQuality and -MaxQuality are explicit opt-in native preset commands. The launcher uses -r4 to select the required DX11 VR backend; it cannot run VR on the original R2 renderer.

The original config currently selects 1920x1080 rather than the former forced 2560x1440. VR eye buffers initialize from the engine render dimensions. Changes that recreate render targets or compile shaders should be assessed after a full game restart.

VR no longer hard-disables motion blur in phase_combine. Previous-view matrices are isolated across left/right game eyes and left/right optics views; initialization/discontinuity frames have no temporal history. DOF is governed by the imported native option.

Validation: imported hashes equal original; subsequent normal PrepareOnly leaves the hash unchanged; R4 Release build succeeded. Game not launched.
