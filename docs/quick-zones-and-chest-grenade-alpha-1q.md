# Quick zones and chest grenade chord (alpha 1q)

- Two adjacent quick-item boxes below chest: combined .44 x .22 x .18m,
  center y=-.535,z=-.04. Left/right halves map left hand to quick slots 1/2,
  right hand to slots 3/4. Shared quickZone predicate drives selection.
- Quick items no longer render/grab on the belt. Grenade also uses chest access
  instead of its prior belt target. Ammo proxy is retained on the belt.
- Fresh grip+index-trigger chord in chest chooses native GRENADE_SLOT and queues
  automatic PrimeVR once native activation is ready. A 90ms grace interval for
  plain chest grips distinguishes the chord from detector/binocular selection.
  Existing held items are never replaced by merely passing through a zone.
- A primed grenade released in chest calls CancelPrimeVR, hides the native held
  grenade, and preserves inventory count. Fuse starts only in native Throw;
  cancellation does not detach/activate the fake missile. Release outside chest
  uses the existing tracked throw. Native unprimed grenade returns remain.
- Temple centers move .19 -> .14m from head center (5cm inward).
- Reload port radius .13 -> .065m. Belt ammo grab volume unchanged.
- Torso yaw deadzone +/-60 -> +/-30 degrees. Head-attached flashlight/weapon
  zones retain headset anchoring; body gear follows the tighter torso heading.
- Debug contours updated: two pink hotbar halves, no old quick/grenade belt boxes,
  corrected temple positions and smaller reload sphere.

Validation: body/zone tests passed including all four hand/zone mappings,
separation from chest, updated torch reach and tighter yaw deadzone.
Native grenade cancellation and gesture timing require headset validation.
No game test launched.

Final validation: xrGame Release x64 build succeeded; engine patch whitespace/reverse checks passed. Grenade chord also consumes F/use; cancelled activation clears its native hand marker.
