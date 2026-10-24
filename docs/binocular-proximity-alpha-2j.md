# Binocular proximity aperture, alpha 2j

Keep the configured 3.9 cm base diameter at distances of 15 cm and greater. Between 15 cm and 3 cm head-centre distance, use smoothstep to add up to 1 cm radius (2 cm diameter), reaching a 5.9 cm diameter at 3 cm and closer. This changes the aperture geometry only; magnification and optical camera projection are unchanged. Both apertures use the same tracked head-centre distance to the lens-pair centre, avoiding different sizes between stereo passes.

Tests cover the start, midpoint, maximum, saturation and invalid-distance fallback. Game launch is not authorized for this change and was not performed. Headset verification remains pending.

The same proximity factor also extends both lenses up to 0.02 m away from the model toward the eyes. The factor uses the original lens anchor, so extension cannot feed back into itself. Both lenses share the same size and displacement. The optical scene camera origin and activation-distance reference remain at the original model anchor, preventing this visual adjustment from moving the viewed scene or cancelling the optical pass prematurely.
Final xrGame build and aperture/extension/zoom tests passed; patch checks passed.
