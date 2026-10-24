# Rectangular chest, narrower rear/torch zones and stereo rain (alpha 1o)

User confirmed alpha 1n behavior works.

Zone changes (full dimensions, torso/head local metres):
- Chest/stow box: .44 wide x .22 high x .18 deep; center y=-.29, z=-.04.
  Shared predicate and rendering dimensions; previously sphere radius .11 at
  y=-.36,z=.02. Center moved 7cm up, 6cm into torso.
- Rear knife/bolt box: width reduced .84 -> .42; height/depth unchanged.
- Each temple flashlight box: width reduced .20 -> .10, centers unchanged.
  Shared templeVolume helper now drives hit detection; same constants draw wires.
- Debug zone outlines remain enabled.

Rain:
- dxRainRender previously advanced each drop and splash lifetime per render call.
  VR renders both eyes, causing two simulation steps for one Device frame while
  collision lifetimes were still based on real elapsed time. This can carry drops
  beyond their roof hit point before native expiry.
- In VR the per-renderer RainStep stamp allows one simulation update per Device
  frame. Both eyes still draw with their own view/culling, using the same drops.
  Birth/expiration, movement, wrap and splash ageing are gated together.
- A drop head reaching its precomputed ray hit point is retired before drawing
  beyond that surface. The second eye also skips that retired drop. Native roof
  ray tests and outdoor rain remain active; no blanket indoor rain toggle.
- Flat rendering retains its prior update behavior.

Validation: chest/back/temple boundaries and symmetry passed; rain step test
checks repeated stereo/render passes and frame-counter wrap. Headset verification
of rain beneath roofs and the revised reach remains pending. No game launched.

Final validation: Game Release x64 build succeeded, including xrGame and R4 renderer; patch checks passed.
