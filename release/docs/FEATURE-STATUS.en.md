# Feature status — Alpha 0.1.0 candidate

Evidence is from the development session on Quest 2 / Touch / USB Link. Historical confirmations are **not** final-build regression acceptance. No complete campaign or multi-headset qualification is claimed.

| Feature | Evidence / remaining work |
|---|---|
| Stereo world, head tracking, both-eye lighting | Explicitly confirmed; Skadovsk doubling/noise and one-eye lighting were reported fixed. |
| Touch aiming, shooting, haptics, left/right grips, hand transfer, support aim | Confirmed during earlier iterations; final per-weapon sweep still needed. |
| Free hands, knife/binocular/detector poses | User reported all grips working after pose corrections; every animation/model variant not certified. |
| World pickup, inspection, transfer, throw; grip-to-equip weapon | Confirmed. Throw strength was subsequently increased; exact range is not measured. |
| UI rays, skip startup logos, loading acknowledgement | Confirmed. Latest setup path and new-game flow must be retested. |
| Stereo inventory/dialogue/PDA overlays | Confirmed at earlier checkpoint. Closing-transition fixes and every PDA page still need regression. |
| Body-zone prototypes, flashlight/head-follow, sleep, head-relative movement and audio | Explicit earlier confirmations exist. Later geometry/calibration changes still need final regression. |
| Physical crouch changes collision height | Explicitly confirmed. New full calibration, speed halving and sprint block are code/build tested, not headset-confirmed. |
| Binocular zoom, stereo view, scope lenses/tube/reticles | Earlier optical result confirmed good; latest near-plane change requires checking again. |
| Physical magazine reload and internal/single-round shotgun/RPG reload | User reported the reload group working before later refinements. Precise contact/sound/visibility fixes, all 26 mag profiles and seven internal profiles require systematic testing. |
| Inventory unload | Confirmed in inventory. Loot/trade support was added later and needs explicit confirmation. |
| Underbarrel loading/switching, all launcher projectile origins | Implemented; later muzzle correction not headset-confirmed. |
| RG-6 pull latch, 180° opening, drum indexing; SPAS open-action requirement | Implemented/refined after feedback; no explicit final confirmation. |
| Jam clearing by full charging-handle stroke | Implemented after user report; final behavior unconfirmed. |
| Firearm + auxiliary bolt | Added without the rolled-back independent-weapon system; latest path unconfirmed. |
| Grenades, consumable activation, four quick slots, ammo-type dragging | Implemented; complete use/cancel/throw inventory-consumption regression pending. |
| Physics crates/bodies, collision suppression | Earlier group confirmation; arbitrary physics props/quest pickups remain experimental. |
| Wrist HUD/minimap, depth/hand overlap, status icons | Implemented and iterated; final presentation/occlusion needs checking. |
| Shadows from obsolete actor/arms model | Multiple fixes, including final tracked palette; latest fix unconfirmed across all lights. |
| No player/NPC physical collision | Latest early-contact correction unconfirmed; hit detection intentionally remains. |
| Graphics import compatibility | Corrected launch confirmed working after a prior black-screen system hang. Graphics snapshot is now the release default. |
| Zone-visibility menu toggle; 5 cm near clip/depth flicker fix | Build and code/unit checks only. No headset confirmation yet. |
| Death → load save | Historical crash reported repeatedly; later changes lack explicit successful tester confirmation. High-priority release test. |
| Installer, EXE launcher, uninstall | Installation and launch confirmed by owner on 2026-10-25; automated extraction/self-check passed. Uninstall/reinstall and every UI workflow are not individually confirmed. |

Not supported: standalone Quest/APK, bare-hand tracking, full-body tracking, simultaneous independent dual firearms, multiplayer, Enhanced Edition, official Steam achievements/cloud-save integration, Anomaly/overhaul compatibility. The independent dual-item experiment was rolled back because it could leave hands unable to equip. Native detector combinations have selective relaxations, not unrestricted two independent item lifecycles.

Prototype semantics: discarded magazines are temporary visuals with shared-pool ammunition accounting. Equipment slots use estimated torso anchors. There is no guarantee of realistic anatomy, physics for every quest prop, individual retained-magazine ammo or full campaign completion.
