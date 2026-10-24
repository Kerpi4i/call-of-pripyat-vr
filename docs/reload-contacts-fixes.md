# Reload contact corrections

- RG6: observed log `RG6 authored opening pose valid=0`. The stock item OMF contains `idle`, not `rg6_idle` (the latter is a hands alias). Fixed the item idle lookup, so native lock/reload opening transforms can be sampled. The asset validator now checks idle and opening names in the extracted native OMF.
- Protecta: the native shell-ejection coordinate used previously is far forward of the loading window. A software-rendered textured inspection of the native mesh places the new port on the receiver window, source coordinates (0.0342, 0.125, -0.010). In-headset alignment remains to be confirmed. Inspection image: reports/model-inspection/protecta-loading-window.png.
- Protecta has no independent shell bone. Its held 12g round now reuses the native SPAS shell reload contact, already logged valid with 6.8 cm authored palm offset. The same anatomical mirroring supports either hand.
- BM16/TOZ34 break contact offset is now +0.30 m toward the muzzle from the neutral foregrip: +0.20 m beyond the preceding +0.10 m.
- Articulated shotgun support contacts are sampled from isolated native idle hands/item models and stored in bind-object coordinates. Current animation/body articulation is applied once at render time. This prevents capturing a knife/show transition as a permanent foregrip position.
- Rear-waist knife extraction is blocked while a firearm in native weapon slot 2/3 is held by the right hand. The opposite right-hand bolt gesture with a left-hand firearm remains available. Reload chord retains priority.

Validation: asset profiles, physical parts and hand symmetry; Release build. No automatic game launch.
