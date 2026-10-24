# Zone placement adjustments, alpha 2d

- Chest and both quick-access regions move forward 4cm (tracking-space forward -0.04 -> 0.00m). Shared chest inventory stow/grenade interactions follow the same chest region.
- Head weapon draw/return volume moves down 10cm and backward 5cm without resizing. New vertical extent [-0.10,0.30)m; draw depth (-0.55,-0.03)m and return depth (-0.55,0.21)m.
- Draw the debug boxes using the same offsets as collision tests.
- Enable body/debug_zones=true and debug_other_zones=false: chest, quick zones and weapon region visible; unrelated flashlight/rear-tool/ammo/reload debugging stays hidden. Functional interactions unchanged except requested positions.

Validation: body_tracking_tests includes translated bounds and passes. xrGame build required. No game launch.
