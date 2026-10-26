# Test-build performance capture / Діагностика продуктивності

Only development binaries are instrumented. No release assets are changed.

## Capture

1. Start `Launch-Test-Profile.cmd` when ready to test. It selects MSAA 4x and 2080x2096 pixels per eye, preserving the saved quality settings. Profiling is opt-in; normal launches do not issue GPU queries or write profiling rows.
2. Load the desired save and wait for loading/initial streaming to settle. At Skadovsk, spend about 30 seconds facing one direction, then 30 seconds looking around. Note approximately when loading finished. Test optics separately so the extra rendering can be identified.
3. Close the game normally to flush the final CSV rows.
4. Run `Summarize-Test-Profile.cmd`. It summarizes the newest capture under `reports/performance/<timestamp>/` and writes `summary.txt` beside `engine.csv` and `bridge.csv`.

For controlled comparisons, use PowerShell from the project folder:

```powershell
.\tools\run-game-vr.ps1 -Profile -Msaa Off
.\tools\run-game-vr.ps1 -Profile -Msaa 2x
.\tools\run-game-vr.ps1 -Profile -Msaa 4x
```

Run only one game at a time. Keep the same scene, headset refresh rate, direction, quality and Link settings. The MSAA selection is a process-only override. To test reduced eye resolution, add `-EyeWidth 1664 -EyeHeight 1677`; keep all other settings unchanged.

Example analysis restricted to a settled interval, starting 60 seconds after capture begins:

```powershell
python tools/summarize-vr-profile.py --skip 60 --seconds 30
```

With an explicit directory, older captures can also be analyzed. The script overwrites only that capture's summary.txt. No automatic bottleneck verdict is generated.

## Recorded stages

Engine CSV:

- `simulation_cpu`: FrameMove, including its internal waits.
- `camera_cpu`: camera update.
- `parallel_wait_cpu`: main-thread wait for background readers before per-eye cameras are installed.
- `parallel_tail_wait_cpu`: final task synchronization.
- `eye0_render_cpu`, `eye1_render_cpu`: CPU wall time in scene rendering for each eye. Includes driver calls and possible GPU backpressure, not GPU duration.
- `optics_render_cpu`: additional optical view rendering, one row per optical pass.
- `bridge_begin_cpu`: bridge entry including runtime pacing and tracking/input.
- `render_cpu_wall`, `process_frame_cpu_wall`: inclusive totals. Do not add them to their constituent stages.

Bridge CSV:

- `xr_wait`: runtime frame pacing, not CPU computation.
- `xr_begin`, `xr_end`: runtime frame calls.
- `swapchain_eye_wait`, `swapchain_ui_wait`: image availability waits.
- `eye0_copy_cpu`, `eye1_copy_cpu`: CPU copy/submission scope, including nested eye swapchain waits.
- `gpu_frame_elapsed`: D3D11 timestamp interval from after xrBeginFrame to before xrEndFrame. Includes optical/world/UI rendering and copies, but also command-stream gaps and GPU preemption. It is not a GPU utilization percentage or pure GPU busy time.
- `xr_period`: predicted display period in milliseconds, a budget value rather than a measured stage.
- Negative-duration records indicate disjoint/unavailable GPU timing or a full query ring, and are excluded from timing statistics.

The GPU query ring uses 16 slots and `D3D11_ASYNC_GETDATA_DONOTFLUSH`. It does not poll in a loop, wait for results or call Flush. If no slot is free, that frame's measurement is skipped. Some final GPU samples can remain unresolved at shutdown; they are deliberately not drained with a blocking wait.

CSV frame IDs belong to their own component. Engine and bridge IDs need not match; QPC timestamps share the same system timebase. Loading/menu/paused frames are included, so analyze a stable interval instead of averaging the entire launch. Logging and query overhead are nonzero; compare a normal run if capture itself appears slower.

## Interpretation

A large resolution/MSAA-dependent GPU interval supports a GPU-side limit. A long simulation/pre-render wait or eye CPU submission time that barely responds to reduced resolution supports CPU/driver investigation. GPU elapsed can itself be inflated by CPU starvation, so it cannot independently establish GPU saturation. Confirm ambiguous results with a runtime performance HUD or a deeper graphics trace.

This capture does not measure compositor dropped frames, ASW state, Link encode/decode latency, or every render subpass. Those need runtime diagnostics or a subsequent detailed capture. Do not interpret xrWaitFrame time as wasted CPU time or assume total CPU utilization exposes a saturated main thread.

## Українською

Запуск діагностики — `Launch-Test-Profile.cmd`. Після завантаження побудь близько хвилини в одній локації, окремо перевір оптику й закрий гру. `Summarize-Test-Profile.cmd` створить зведення останнього запису. Для висновків потрібно відкинути меню та завантаження і порівняти однакову сцену з різними MSAA/роздільністю. Збірка сама не визначає винуватця за одним числом FPS.

## Detailed renderer capture

The next capture also includes renderer.csv. Stage names have eye0/eye1/optic0/optic1 prefixes. Renderer CPU scopes include calculate_total, visibility_build, gpu_sync_and_visibility_wait, scene_prepare, geometry_primary, geometry_secondary_decals, light_visibility, msaa_edges, rain_sync, sun, emissive, local_lights_normal/pending, local_shadow_draw_and_wait, local_shadow_task_wait, combine, forward_transparency, ssao/hdao, bloom and postprocess. Optional stages are recorded only when executed.

Matching GPU intervals appear in bridge.csv using the same names. They use timestamp markers inside the existing frame disjoint query, not a new disjoint query per stage. No GPU marker is issued from worker-thread visibility calculations or CPU-only wait scopes. The marker capacity is 256 per captured frame; overflow produces gpu_marker_capacity and omits excess intervals. Queries are allocated lazily, so discard the first settled seconds of a capture to avoid initial allocation effects.

Scopes can be nested: combine includes some forward rendering, SSAO and bloom; local-light scopes include local shadows. Do not sum parent and child scopes. Repeated local-shadow rows are per batch, not per frame; sum by component/frame/stage when estimating the total per-frame cost. Deferred-context command recording can make the shadow GPU interval reflect only immediate-stream work; read CPU timing and the enclosing local-light interval alongside it.

Each new capture saves capture.json with launch parameters and a copy of the graphics config. Actual effective MSAA is also logged by the renderer. Older captures do not have this metadata.

Builds and offline CSV/statistics tests passed. Detailed GPU stage capture remains to be verified in a user-authorized game launch. Use the same Launch-Test-Profile.cmd; no additional launcher is required.
