# Development test — eye resolution and legacy sun coverage

**Regression rollback:** the tester reported stretched translucent polygons on the location after this combined change. The legacy-sun projection modification has been reverted, test AA/texture/filter values restored from backup, and the launcher once again defaults to saved `vid_mode`. Explicit eye-size parameters remain experimental and opt-in. The rest of this document records the attempted implementation, not the current default. No headset confirmation of the rollback yet.

Local only; no release assets, release patch, tags or remote were updated. No game launch.

The development launcher now passes `-copvr_eye 1920x1920`. Engine `Device_mode.cpp` keeps `psDeviceMode` as desktop-window size and uses the requested dimensions for the DX11 render surfaces / OpenXR eye buffers. Thus changing the desktop window no longer dictates eye-buffer aspect ratio. DXGI presents the existing backbuffer into the desktop window; this is a stretched mirror, not an additional scene render. Explicit test parameters: `tools/run-game-vr.ps1 -EyeWidth 1832 -EyeHeight 1920`. Process restart required. Valid dimensions: 512–4096, then checked against OpenXR maximum dimensions during bridge initialization.

1920×1920 has the same pixel count as 2560×1440: 33% more vertical samples, 25% fewer horizontal samples. This is a same-budget near-square diagnostic setting, not a guarantee of panel-perfect output or the OpenXR runtime's recommended resolution. It does not change Link encode settings, headset refresh rate or codec quality. Future automatic recommended-size selection remains separate work.

Stock CoP DX11 `accum_sun_near.ps` uses `v2p_TL2uv`; the engine detects that shader layout and selects legacy sun cascades even with `r2_shadow_cascede_old off`. That path rebuilt three frusta from flat `Device.fFOV/fASPECT`, inconsistent with the off-axis VR projection. All three now preserve the current eye projection's angular scale/offset and near plane, changing only the far depth range. Non-VR retains the original construction. The new cascade path already reads the actual inverse full transform. Render initialization logs the selected sun path for verification.

Test config was backed up under `reports/backups/test-clarity-*`. Enabled native post-process `r2_aa`, restored full texture quality (`texture_lod 0`) and anisotropy 16. Original AA kernel/weights/barriers retained. MSAA remains off. Full textures may increase VRAM use; a square buffer and post AA do not guarantee foliage stability or remove Link compression artifacts.

Validation: geometry tests cover valid/invalid startup dimensions and off-axis projection preservation, near/far depth mapping. Projection reconstruction, binocular optics and UI filter regressions pass. x64 Game/R4 build passed. Still needs headset comparison: vertical detail, menu/optics layout with square buffers, shadow visibility during head rotation and performance.

Engine changes not included in the frozen release patch: `engine/src/xrEngine/Device_mode.cpp`, `engine/src/Layers/xrRender_R2/r2.cpp`, `engine/src/Layers/xrRender_R2/render_phase_sun_old.cpp`. Source helper: `src/vr_render_geometry.hpp`.
