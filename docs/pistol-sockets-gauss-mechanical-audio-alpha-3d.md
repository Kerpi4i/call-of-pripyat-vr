# Pistol sockets, Gauss readiness and native mechanical audio, alpha 3d

Pistol magazine extraction/insertion socket centres move 4 cm down in native weapon model Y. The magazine mesh centre is unchanged: socket and visual centre are now separate, including captured detachment pose and the lower contact point of fresh held pistol magazines. Rifle sockets remain at their measured native centres.

Gauss magazine profile sets manual_rack=false. Insertion makes it ready immediately and completes jam recovery for that profile. VRCanFire also accepts an already installed Gauss magazine in older alpha-3b/3c saves whose pending-chamber flag remains set; a missing magazine and native misfire still block firing. Other magazine-fed guns retain manual chambering and jam-reload requirements.

## Native action sounds
Read-only extraction of local resources.db0 located the actual native reload recordings, including combined magazine and charging sounds. tools/build-vr-mechanical-sounds.py creates 41 short mono 44.1kHz Vorbis excerpts for 33 HUD profiles from their configured reload audio and existing standalone native closing sounds. Per-source out/in/rack time ranges are recorded in reports/mechanical-sound-cuts.json. Shared native reload recordings remain shared, matching the stock configs. Five-millisecond boundary fades avoid hard-cut clicks. Original binary X-Ray Vorbis distance, volume and AI metadata is restored in the new comment packet and its Ogg page CRC recalculated; no Steam archive is changed.

CWeaponMagazined lazily loads three independent short action sounds. Removal, insertion and successful racking play their own excerpt at the tracked HUD item position as 3D audio, following that position until natural completion. Stop/destroy at net teardown. The non-detachable manual charging path requests native reload accounting with the full soundtrack suppressed, using only its rack excerpt. Original non-detachable belt-activator and grenade reload fallback behaviour otherwise remains. Ordinary full reload animations remain disabled; dedicated jam clips retain alpha-3c behaviour.

Excerpts were selected from separated native transients and quiet gaps. They are not yet confirmed by an in-headset listening test, particularly multi-step PKM/launcher recordings; timestamps are explicit and adjustable.

## Empty pistol magazine visibility
All ten native pistol HUD OGF files have local overrides splitting magazine triangles into independent skeletal children. Original geometry, vertex attributes, materials, skeleton, motion data and all parent chunks are preserved; only child batching changes. Static skeletal children avoid invalid progressive LOD indices after the split. Magazine children have a single native cartridge/joint4 bone so native visibility pruning excludes them in both loaded and empty-slide animations. This does not remove parts of the structural grip that stock assets skin to the body bone.

## Validation
xrGame Release x64 build succeeded. Magazine transition tests include no-rack Gauss insertion. Offline checks validate 26 magazine assets, ten exact triangle/material-preserving pistol splits with original animation chunks, 4 cm sockets, Gauss profile and 33 part contacts. tools/validate-vr-mechanical-sounds.py verifies all 41 excerpts decode, mono 44.1kHz format, cut durations, page CRCs and unchanged native binary metadata. Engine diff and reverse patch checks pass. No game launch; reachability, empty-pistol appearance and subjective action sound timing require headset confirmation.
