# Physical magazine reload prototype, alpha 3b

## Interaction
- Grip the installed magazine with the free hand (palm within 6.5 cm of its measured native mesh centre). The actual magazine bone disappears and its extracted model follows the hand. Release grip to discard it.
- Reach to the rear waist with the opposite hand and grip + index trigger to obtain a fresh magazine, replacing the old ammo-box activator for detachable-magazine firearms.
- Move the held magazine into the same weapon socket while holding grip: insertion occurs immediately, without release. Releasing elsewhere returns the unused activator.
- Grip the native charging handle/slide, pull backwards and release. Pistol travel is 3.5 cm; rifle travel defaults to 7.5 cm. A newly seated magazine cannot fire before this gesture. Clearing a misfire through the handle does not refill a detachable magazine.
- Canned reload/cocking clips remain disabled. Native non-detachable/tube/grenade-launcher reload accounting retains the previous no-animation fallback.

## Coverage and implementation
26 HUD profiles: all 10 stock pistols and 16 magazine/box-fed long guns, including Gauss battery and PKM ammunition box. Rifles commonly use magazin; pistol magazines use cartridge, Browning joint4. Browning slide is joint2, other pistol slides breachblock. Reproducible extractor tools/build-magazine-models.py exports only magazine triangles using original UV/material/tangent/skin data, plus rigid native skeleton metadata. No substitute pistol mesh is used. Read reports/magazine-models.json for measured centres and bone mappings.

Per-weapon present/needsChamber state is saved behind an optional RVM1 marker, with compatibility reading older saves without the marker. FireStart and FireTrace enforce it; ordinary reload actions cannot bypass physical insertion. Underbarrel mode retains its independent rocket ownership path. HUD magazine visibility reflects the state. Native inventory ammo-type selection is applied at insertion; loose world/inventory ammo boxes cannot trigger insertion.

Prototype limits: remaining rounds are returned to the native shared ammo pool upon removal, rather than retained individually in discarded magazines. Discarded magazines are temporary visual objects (24 maximum, 30-second lifetime), with gravity and static-world ray contacts, no actor collision, inventory entry, persistent ammo or re-grab. Fresh magazines use native loose ammunition on seating, not on acquisition. Native dropped weapon world models have not received per-bone magazine visibility profiles. Handle gestures gate chambering but the bolt mesh itself does not yet follow the physical hand travel.

## Test copy
Load copvr_all_weapons_test. This is a separate copy of the latest non-test save, with thumbnail, and the source is backed up in reports/backups/all-weapons-test. Its named-load kit issues all configured stock wpn_ weapon sections (including unique variants), and F1/RGD5 if absent. It tops up each compatible ammunition pool to 100 rounds per corresponding weapon section, summed for shared calibres; knife/binocular dummy ammo classes are excluded. Existing amounts above the target are kept. Weight limits are raised only for this named test copy. Items are issued on load, not already serialized into the copied source file; save in-game after the kit appears to persist it. Reopening that named test copy replenishes missing kit entries and spent reserve ammunition. Original saves are not changed or issued kits.

## Validation
Release x64 xrGame build and pure MagazineReload transition tests; offline validation of all 26 rigid magazine models, native vertex sizes, indices and profile mappings. No game/headset launch. Visual alignment, grab reachability, native mesh rendering and gameplay need headset confirmation.
