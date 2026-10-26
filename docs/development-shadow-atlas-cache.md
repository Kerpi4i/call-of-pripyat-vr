# Experimental same-frame local shadow reuse

Enabled only by `-copvr_shadow_cache` / `tools/run-game-vr.ps1 -ShadowCache`. Ordinary Launch-Game-VR.cmd remains unchanged. Launch-Test-Profile.cmd now enables this experiment; Launch-Test-Profile-Baseline.cmd selects the identical MSAA 4x/profile settings without it. No release files or published patch are updated.

## Method

The local-light renderer packs lights into atlas batches, clears the shared depth atlas, builds caster lists, renders shadow depth and accumulates light. Later views rebuild the same work. The prototype keeps a depth-slice snapshot for an exact batch within Device.dwFrame, then restores it on a match, skipping caster collection and depth drawing. Light accumulation still runs independently for each eye/view.

Keys include the ordered light identities, atlas rectangles, light shadow matrices, current screen/LOD scale, FOV and light-render flags. Source texture replacement and simulation-frame changes invalidate entries. Different packing, projections or visible light lists miss the cache and use the original rendering path. Cached contributors retain the original distinction between an empty shadow pass and a light which should be accumulated.

The prototype supports only single-sample, one-mip typeless depth textures (R16/R24G8/R32), copies slice zero and falls back when unsupported or allocation fails. It bypasses caching when translucent shadows are enabled. Storage is bounded to eight snapshots and 128 MiB of depth storage. Once full, unmatched batches render normally. No cross-frame reuse occurs, so moving objects are not intentionally frozen.

The atlas is unbound before copying, and backend state is invalidated. No CPU readback or explicit GPU wait is used in the production cache. Copy/restore bandwidth and initial allocation are additional costs; speedup is not assumed.

## Verification

Native Game target builds. A software D3D11 WARP test verifies actual depth-array slice copying/restoring, key mismatch, contributor recovery, frame invalidation and storage capacity. This test creates no window, OpenXR session or game process. It uses a blocking staging readback only inside the test to verify copied depth values.

Profiling exposes shadow_cache_lookup_restore, shadow_cache_store and CPU shadow_cache_hit, with the standard eye/optic prefixes. Compare hit counts and total local-shadow cost per frame. Ordinary geometry, lighting, MSAA, and graphics preset are not reduced by the experiment.

Headset verification is still required: grate shadows while pitching the head, light visibility in both eyes, moving NPC/flashlight shadows, and binocular views toward/away from Skadovsk. Exact matching may produce few hits if packing changes between views; this is a safe miss, not a reason to relax the key without further analysis.

For a test, launch Launch-Test-Profile.cmd, repeat the previous route, then close normally and report completion. If artifacts appear, close the game and use Launch-Test-Profile-Baseline.cmd. No automatic game launch was performed during implementation.

## Miss classification

When profiling is enabled, the exact key is also split into context (screen/LOD scale, FOV and flags), sorted light membership, per-light sizes, per-light shadow transforms and atlas layout/order. Matching still uses the original exact key. Diagnostic metadata does not relax cache validity.

The renderer CSV now records shadow_cache_miss_cold/members/context/sizes/transforms/layout/other plus shadow_cache_store_failed. The closest valid same-frame entry is selected by weighted matching: membership, context, size, transforms, layout. The first mismatch in that order is reported. This is a diagnostic comparison against the closest retained batch, not proof that no other field changed. No entries means cold; storage failure is recorded separately and may mean capacity/allocation failure.

Metadata construction and miss classification are skipped without -copvr_profile. WARP tests cover all five mismatch categories, along with the existing depth copy/restore and invalidation checks. No new game capture has yet verified the distribution of causes.

## Frame-stable adaptive sizes

Capture 20261026-172151-850 found 548 right-eye size mismatches, 422 membership mismatches, one layout mismatch and 171 hits. This prompted a first isolated change: each light now owns a FrameShadowSize state. With -copvr_shadow_cache only, the first computed adaptive size is retained for all later calls in the same Device.dwFrame. The next frame again evaluates the existing adaptive formula/hysteresis. Different lights have independent state; image-cache keys remain exact and retain all prior validity checks.

This uses the first view's computed size, not a precomputed maximum over both eyes. It prevents repeated within-frame adaptation without introducing a separate stereo light-collection pass. The effect on edge-case shadow detail and the actual hit rate needs headset verification. Baseline/no-shadow launchers do not enable this sizing change.

Offline tests cover repeated candidates, independent lights, new frames and frame-number wrap. Use Launch-Test-Profile.cmd for the next capture and compare shadow_cache_miss_sizes/hit counts plus local-shadow time. Do not assume that resolving size changes also resolves atlas membership differences.

## Subset reuse prototype

The next test retains full depth-slice snapshots but also records exact per-light tile keys. After an exact batch miss, a saved atlas can be reused if it contains every requested light tile with identical identity, matrix, size, position and context. The saved atlas may contain additional lights, or a different ordering. Only requested lights from the saved contributor list are accumulated.

This is not arbitrary tile relocation or mixing tiles from multiple snapshots. If a requested light is missing, moved, resized or transformed, the original shadow path remains. This conservative step handles compatible subsets without adding a depth-blit shader or relaxing exact per-light matching.

Because PCF can read adjacent pixels, extra populated rectangles within a 16-texel margin of any requested rectangle reject the subset restore. This margin is a conservative prototype guard, not a replacement for visual testing of all custom shadow shaders. Extra empty/noncontributing tiles do not trigger this guard.

Profiling records shadow_cache_subset_hit as a subset of shadow_cache_hit; do not add the two counts. Frame-stable adaptive sizes, same-frame invalidation, eight-snapshot/128 MiB limits and baseline launch behavior remain unchanged.

WARP tests cover subset lookup, reordered requests, contributor filtering, context mismatch, changed rectangle keys, missing lights, and rejection near an extra populated tile. The engine build also passes. The effect on FPS and any shadow filtering artifacts require the next headset capture via Launch-Test-Profile.cmd.

## GPU tile relocation prototype

Launch-Test-Profile.cmd now adds -copvr_shadow_tiles (and the base cache flag). Launch-Test-Profile-Atlas.cmd retains the preceding whole-atlas/subset implementation; Launch-Test-Profile-Baseline.cmd has neither cache. Normal launchers remain unchanged.

After exact/subset misses, individual light tiles are searched across retained same-frame snapshots. A separate content key includes light identity, size and shadow matrix, excluding atlas X/Y. Context/LOD/FOV/render flags must still match. Matched tiles are copied into the current atlas rectangle by a tiny depth-only GPU draw using integer Texture2D.Load and SV_Depth. Unmatched lights are rendered normally. Partial hits skip only matched lights, and contributor/empty-pass status is retained per light.

Snapshots remain whole single-slice textures, now with SRVs, within the existing eight-snapshot/128 MiB budget. No cross-frame reuse, CPU readback, explicit GPU Flush or blocking query is added. A depth-copy shader is compiled lazily through the local d3dcompiler_47.dll; initialization failure falls back to ordinary shadow rendering. Initial compilation/allocation can cause a one-time profiling spike.

The depth blit is tested with software D3D11 WARP in R32, R24G8 and R16 formats: two light tiles from different snapshots are relocated to different coordinates, missing/changed tiles miss, expected depth values are read back, and surrounding pixels remain unchanged. Readback occurs only in the test. The copy does not transfer stencil, which is not used by shadow-depth sampling.

This experiment copies nominal tile rectangles, without the whole-atlas subset path's extra-neighbor margin. Filtering at tile borders can therefore differ and explicitly needs headset verification: thin grate shadows, shadow edges, both eyes, head pitch, flashlight and moving NPCs. No visual equivalence or FPS improvement is claimed before that capture.

New profile stages: shadow_tile_restore measures lookup/blit cost; shadow_tile_hit counts restored lights (not atlas batches). shadow_cache_hit still counts fully restored batches and can overlap tile hits; do not add these counts. A partially restored batch continues to record its original batch-miss reason, followed by rendering of uncached lights.

## Shadow preparation pipeline experiment

The previous implementation queues caster-list jobs up to the renderer context limit, then drains the entire queue in insertion order before refilling it. The opt-in -copvr_shadow_pipeline variant chooses an already-finished job first. When the context pool is full, it draws/releases just one job, then returns to scheduling the next light while remaining jobs can continue. At the atlas-batch end it drains all remaining jobs. If none is ready, it waits on the oldest job through the existing TaskScheduler::Wait, which can execute queued tasks on the caller thread.

Each job is waited on before reading its graph; each context is released after its depth drawing and reused only afterward. Shadow tile dimensions, matrices, caster selection and per-frame update frequency are unchanged. Independent atlas rectangles can be drawn in completion order. Accumulation order may differ slightly as a result; verify lighting and shadows visually.

Launch-Test-Profile.cmd now enables this pipeline together with tile reuse. Launch-Test-Profile-Tiles.cmd retains the preceding tile implementation without the pipeline. Other earlier launchers retain their flags.

CPU profiling exposes local_shadow_pipeline_release for incremental context releases. Compare first-eye render, local_shadow_task_wait, total local-shadow cost and frame-time tails; increased release counts alone do not prove speedup. Queue-selection/drain tests, GPU-cache regression tests, Game build and PrepareOnly verification pass. No game was launched; runtime speedup and visual correctness remain unconfirmed.

Capture 20261026-185745-704 exercised the pipeline (95184 incremental releases) and recorded optical passes. Across active non-optical frames, median stereo task wait was 0.428 ms versus 1.940 ms in the prior tile-only capture. Routes differ, so this is supportive evidence, not a controlled FPS comparison. The full-frame median remained near 27 ms; optical p95 was about 56 ms. 602 frames exhausted the previous 256 GPU-marker capacity: whole-frame GPU timing is still present, but detailed stage coverage is incomplete in those frames. The bridge marker capacity was raised to 1024 for subsequent captures; shader/cache/render quality are unchanged by that diagnostic-only adjustment. The higher capacity itself is build-verified, not yet capture-verified.

## Conservative point-face visibility experiment

-copvr_shadow_face_cull bounds the ordinary OMNIPART direct-light draw mesh using all 82 du_sphere_part vertices. Its local AABB is transformed with the same light matrix used by draw_volume and expanded by 5 cm plus 1% of light range. A face is removed before shadow sizing/packing only if the current view frustum rejects this enlarged world AABB entirely.

It does not use the engine's smaller approximate per-face sphere. Intersection/uncertainty retains the face. Volumetric light, enabled GI, custom point/spot shaders, ordinary spotlights and invalid bounds retain the previous path. Every eye/optical view uses its own existing ViewBase; no previous-frame visibility result is retained. The cull skips both shadow building and accumulation for the offscreen face, because its direct-light volume is outside the view.

Launch-Test-Profile.cmd now includes this flag. Launch-Test-Profile-Pipeline.cmd preserves the immediately previous pipeline/tile variant without face culling. Profiling adds shadow_face_culled (per face); the test must establish whether enough work is removed to offset the bounds checks.

Game build, existing GPU-cache/queue/statistics regression tests and PrepareOnly verification pass. No headset test or FPS improvement is claimed yet. Repeat the Skadovsk/outdoor/binocular route, especially looking up/down and across screen edges, and verify no lamps, light patches or shadows pop in either eye. The conservative geometric argument does not replace testing the complete lighting pipeline.
