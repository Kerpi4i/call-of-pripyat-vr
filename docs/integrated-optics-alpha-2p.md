# Integrated optics and second isolated test save, alpha 2p

Extend the physical stereo optics pipeline to permanently mounted sights. Read the reticle from the weapon's native section for permanent optics; detachable optics continue to read the attached addon. Avoid GetScopeName on permanently scoped weapons, where no detachable scope-section list is guaranteed. Reuse the accepted black tube, fitted reticle and near-eye guards, without adding proximity animation or adjustable zoom to rifle sights.

Eight integrated profiles, measured from native HUD vertices: F2000, G36, Gauss, L85, RPG-7, SVD, SVU, Vintorez. All use a fixed lens 8 mm behind the measured rear rim into the model. F2000 and G36 eyepiece vertices are bound to body rather than wpn_scope; a per-profile scope_bone value selects body for those two. Other profiles use wpn_scope. Apply current-bone times inverse-bind transformation as with detachable sights. On G36 the lens uses the lower magnified optical channel, not the upper reflex channel.

Reticle mapping follows native configuration: PSO for SVD/SVU/Vintorez, SUSAT for L85/Gauss, G36 for G36/F2000, RPG for RPG-7. New G36/RPG shader variants bind the original native textures with the existing common tube/reticle shaders. Existing detachable optics and binocular mask behavior are preserved.

## Test save

The latest non-copvr_ .scop save and preview were backed up under reports/integrated-scope-save-backup, then copied to game-vr/appdata/savedgames/copvr_integrated_scope_test.scop and .dds. The .scop copy is byte-identical to the original. Loading it queues an integrated-optics kit through native spawn packets; this preparation did not launch the game or pre-save spawned items.

Manifest reports/integrated-scope-test-kit.json lists thirteen native sections: wpn_fn2000, wpn_fn2000_nimble, wpn_g36, wpn_g36_nimble, wpn_gauss, wpn_l85, wpn_rpg7, wpn_svd, wpn_svd_nimble, wpn_svu, wpn_svu_nimble, wpn_vintorez, wpn_vintorez_nimble. Runtime enumeration requires scope_status=1 and a supported HUD profile, excluding binoculars. Already owned scoped sections are skipped. No detachable-addon flag or AK mount upgrade is applied to these permanent sights.

[integrated_scope_test] in copvr_equipment.ltx independently controls this save and its temporary 10000 kg carry/walk limits. The original [scope_test] still controls copvr_scope_test and its detachable kit. Ordinary saves receive neither kit nor weight override.

Validation: all eight integrated HUD profiles cover the thirteen native variants; native reticle assets/configuration verified; save bytes compared with source; existing optics/zoom/proximity/tube/shared-mask tests passed. Headset alignment, all live spawn variants and actual rendering require user verification. No game launch.
xrGame build succeeded; engine patch whitespace and reverse-application checks passed.
