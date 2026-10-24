# RG6 physical drum reload alpha

Implemented prototype (internal kind 5):

- Empty offhand, fresh lower-grip press at the dedicated latch contact toggles opening/closing. Contact starts at the native lock mesh center (roughly 0, -0.047, 0.114 m in HUD coordinates), radius 5.5 cm. Debug color amber. Ordinary foregrip stays separate.
- The front assembly and lock interpolate between native idle and the final native rg6_reload_start pose over approximately 0.36 seconds. No vanilla reload callback runs. Fire is blocked from opening request until completely closed.
- Rear-waist grip+trigger supplies one full-size native VOG-25 mesh. Release at the green forward chamber contact while fully open to load one round. Release elsewhere returns the inventory proxy without spending ammunition.
- There are six persistent occupancy bits. Loading skips occupied chambers and indexes to the next empty one. Loaded projectiles are rendered separately in the drum, each with a distinct render instance. The original single native feed mesh is hidden.
- Closing chooses the next loaded chamber; no separate cocking action. A partial drum can fire. Shots clear one occupancy bit and advance to the next live chamber. Reopening preserves unspent rounds.
- Each accepted insert consumes one native ammo cartridge and requests one native RG6 rocket entity. VR fire guards run before the native launch path so an open/empty weapon cannot launch a rocket through a rejected inherited fire call.
- RVI1 save extension size remains 17 bytes: cursor occupies unused upper flag bits; live mask expands to six bits. Older saves remain readable. Restored occupancy is reconciled against the native ammo count.

Native model bones were verified: body, lock, reload, rotategrenades, grenade. Opening snapshots are sampled on an isolated temporary model and cached, leaving live animation state untouched. Hand orientation uses the native rg6_reload grip sampler added to reload-grip profiles.

Validation: pure internal reload tests cover six inserts, full rejection, open/transition fire blocking, partial loading, top-up without losing live rounds, next empty/live selection, complete firing. Seven asset profiles and 57 native audio clips validate. Release build and visual headset results recorded separately. Game is not launched automatically.
