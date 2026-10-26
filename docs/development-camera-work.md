# Redundant base-camera work experiment

Launch-Test-Profile.cmd now uses tile reuse + shadow pipeline + -copvr_skip_base_camera_work, without point-face culling. Launch-Test-Profile-Pipeline.cmd is the previous pipeline/tile baseline. Launch-Test-Profile-FaceCull.cmd preserves the rejected/unconfirmed face-culling experiment.

CRender::OnCameraUpdated normally dispatches monocular HOM and detail-cache/visibility work for the actor camera. The existing VR entry point waits for that work, disables HOM (a monocular occlusion mask is not valid for both eyes) and rebuilds detail visibility after installing each eye or optical camera. Under the new diagnostic flag only, the initial unused dispatches are skipped, HOM remains disabled and task pointers are cleared. Calculate now checks for a null HOM task before waiting. Per-view camera/frustum refresh and detail dispatch/wait remain unchanged.

This avoids one base-camera HOM job and one base-camera detail job per callback, not the required eye/optic jobs. All work remains frame-current; no cross-frame vegetation or shadow reuse is introduced. The actor-camera frustum update itself remains. The flag is restricted to the DX11 branch and intended only for the VR test launcher; it is not a supported flat-rendering mode.

Engine CSV adds base_camera_readers_wait_cpu, eye0_camera_sync_cpu, eye1_camera_sync_cpu and optic_camera_sync_cpu. They expose camera synchronization/detail refresh costs outside seqRender, previously included only in render_cpu_wall. Existing stage totals are inclusive; do not add nested stages indiscriminately.

Game build, PrepareOnly validation, statistics tests and GPU shadow-cache/queue regressions passed. No game was launched. Next capture must check grass/detail coverage while rotating/pitching the head and looking through binoculars, as well as performance. The optimization's size and visual correctness remain unconfirmed in-headset.
