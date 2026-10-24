# Reload proxy grips alpha 3g

Rear-waist reload proxies now use an object-to-palm contact sampled from their own native reload motion, rather than a shared aim-axis transform. Source profiles cover 26 detachable magazines and Winchester, SPAS, BM16, TOZ34 and RPG7. Native cartridge/rocket geometry centers are measured from the matching HUD mesh; detached proxy geometry retains its own render center and full size.

A temporary copy of the native hands and weapon skeleton is sampled between 15% and 64.5% of the reload motion. The nearest authored left-palm contact is cached per weapon HUD and hands section. Translation and rotation are expressed in anatomical palm coordinates, then mirrored with a proper rotation for right-hand use. No live animation, sound, ammo or weapon state is changed. Models used for sampling are immediately released. Finger curl remains the existing tracked grip behavior.

The magazine insertion socket no longer shifts a newly drawn magazine away from its held center. Insertion detection remains at the original tracked-palm interaction point. Magazines physically removed from a weapon retain their existing continuous, no-snap attachment.

If an alias, bone, native animation or a plausible contact within 16 cm is unavailable, the proxy retains a neutral centered pose. A successful or rejected contact sample emits `CoPVR: reload grip sampled` with HUD name and palm gap. Cold sampling happens on first extraction for each weapon/hands combination.

Validation: xrGame Release x64 build succeeded; internal-reload asset and physical weapon-part validators passed; hand symmetry tests passed. Native pose sampling and visual alignment require the next authorized headset test. Game was not launched.
