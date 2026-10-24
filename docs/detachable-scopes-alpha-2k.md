# Detachable scope lens prototype and isolated test kit, alpha 2k

Add one collimated stereo optical aperture to each of the seven native detachable-scope HUD models. Use the binocular optical render targets and per-eye cameras. Only an attached, visible detachable scope enables this rendering; detaching it removes the aperture. Permanent integrated sights are outside this change. Scope apertures use fixed model-measured positions/radii and the fixed initial optical field (15 degrees), with no right-stick magnification control or proximity size/extension animation. No reticle or scope-specific night vision/detection effect is added in this prototype.

Profiles in copvr_equipment.ltx are keyed by HUD section, so native weapon variants share their model's calibration. Rear ocular rim vertices were measured from the original local OGF files, selecting vertices belonging to wpn_scope. The aperture is 2 mm behind that rim toward the eye. Convert the model-space calibration through current scope-bone transform and inverse bind transform, so the aperture follows skeletal weapon animations and either-hand tracked weapon pose.

Supported model families: Abakan, AK-74 (with scope-mount upgrade), Groza detachable-scope variants, LR300, Protecta, SIG550 detachable-scope variant, Val. Existing binocular proximity animation and adjustable zoom remain unchanged.

## Test save

The latest native .scop save was backed up under reports/scope-test-save-backup and copied byte-for-byte to game-vr/appdata/savedgames/copvr_scope_test.scop, along with its preview. The binary save itself was not edited. Loading copvr_scope_test with [scope_test] enabled queues the scoped test kit directly into the actor inventory through native spawn packets. The AK-74 receives the native up_fiftha_ak74 scope-mount upgrade in its spawn data. Scope attachment is set before packet serialization. Already owned scoped sections are skipped. Issuance is once per actor spawn, reset on load, and restricted to this exact test save name.

The native configuration manifest is reports/scope-test-kit.json. It contains nine weapon variants: wpn_abakan, wpn_ak74, wpn_groza_nimble, wpn_groza_specops, wpn_lr300, wpn_protecta, wpn_protecta_nimble, wpn_sig550_luckygun, wpn_val. Runtime enumeration applies the same native HUD/scope-status checks.

Temporary carry/walk limits of 10000 kg are returned only for the named test save while enabled. Normal saves retain their existing weight rules. Disable [scope_test] enabled or unlimited_weight in copvr_equipment.ltx and restart to remove the corresponding test behavior. Items added in gameplay persist only if the player saves them; this preparation has not run the game or saved spawned items.

Validation: configuration checks cover all seven models and nine listed variants; test-save bytes match the source; existing binocular optics/zoom/proximity tests pass. Headset position, scope rendering and live kit issuance require verification in game. No game launch.
Final xrGame build succeeded; engine patch whitespace and reverse-application checks passed.
