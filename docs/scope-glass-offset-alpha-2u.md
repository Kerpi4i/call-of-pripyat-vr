# Offset from actual internal glass polygons, alpha 2u

Correct alpha 2t's reference surface: the previous utility measured the outer rear rim. This version intersects the optical-centre ray with actual native mesh triangles belonging to the optical bone (body for F2000/G36, wpn_scope otherwise). Annular rim triangles do not cover that ray. Select the first near-perpendicular internal cap from the player side and position the VR lens 5 mm toward the player from its interpolated depth.

All 15 integrated/detachable profiles updated. Preserve X/Y, diameter, bone attachment, tube, reticle and accuracy settings. This changes model-local Z calibration; it does not rotate the optical camera or alter the existing close-eye guard. Original polygon mesh ID, face index, material and candidate intersection depths are recorded in reports/scope-lens-glass-depth-5mm.json. Prior configuration backed up in reports/scope-lens-depth-before-glass-5mm.ltx.

Reproducible check: python tools/calibrate-scope-lens-depth.py. All 15 profiles verified within one micrometre after decimal rounding. Configuration-only change, restart required; no rebuild or game launch.
