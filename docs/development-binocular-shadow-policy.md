# Binocular-only shadow suppression

The optics view query now returns a positive kind: 1 = weapon scope, 2 = binocular. The view structure layout/ABI stays unchanged. Engine optical pass selection carries the kind in bit 0x100, with low-byte eye values 1/2. The renderer strips the kind bit before indexing eye textures and clears binocular state when leaving the optical pass.

By default, only binocular optical passes use unshadowed local-light accumulation and skip sun caster drawing, preserving illumination. Main eye rendering and weapon scopes retain their prior shadow behavior. Explicit global no-shadow diagnostic flags continue to work as before. SSAO, static/baked shading and cloud modulation remain; this does not remove every dark region in the binocular image.

Sun projection/culling setup is still performed, as in the earlier no-shadow diagnostic; depth clears and lighting remain. Local caster construction/packing/drawing is bypassed for the binocular view. Main-view shadows are still rebuilt normally, and the same-frame light-size/cache code remains enabled under its existing flags.

Add -BinocularShadows to tools/run-game-vr.ps1 to restore binocular shadows for an A/B capture. This is a launch-only flag and does not alter user.ltx. The ordinary development launcher also uses the binocular-specific default after this build; published release binaries are unchanged.

Profiling retains optics_render_cpu and adds nested binocular_render_cpu/scope_render_cpu stages. Do not add the nested times. The latter identify which kind was actually rendered, allowing the next capture to verify isolation.

Offline policy tests cover binocular-only suppression, normal-view retention, explicit global sun/local switches and the A/B override. Native Game build is required before testing because both xrGame and xrEngine interpret the optics query kind. No game is launched by implementation or PrepareOnly validation.

Headset check: inspect the binocular toward/away from Skadovsk, lower it and verify the world still casts shadows, then use a weapon scope to verify its shadows are retained. Capture through Launch-Test-Profile.cmd to measure actual benefit; no FPS improvement is claimed before this test.
