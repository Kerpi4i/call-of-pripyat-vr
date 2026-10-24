# Near-eye optics clipping, alpha 2n

VR eye projections used a 5 cm near plane shared by tracked hands/items and the world. This clips the sight/binocular housing and its optical aperture before it reaches a natural eye relief. Change only the headset eye projections to a 5 mm near plane; retain the optical scene camera's 5 cm near plane and desktop behavior. All headset depth-tested geometry continues to share a single projection/depth range, preserving wrist-panel occlusion fixes.

Reduce optical activation/eye-box minimum axial relief from 5 mm to 1 mm and activation minimum distance from 15 mm to 1 mm. A visible aperture near the eye is constrained to at least 10 mm along its optical axis, above the headset near clip distance. If an eye crosses the physical lens, keep a black opaque stop for up to 25 mm penetration within a 7 cm lateral neighborhood, rather than exposing the model interior through a vanished lens. This is a local optical rendering guard, not physical collision/pushback of weapons or full head/world collision prevention. Extreme oblique angles or deep penetration can still require further comfort handling.

The original aperture location remains the activation and optical camera reference. The near-eye guard changes only the displayed aperture position. Existing scope tube/reticle and binocular proximity size/movement remain in place.

Validation: tests cover 3 mm eye relief, guard displacement on approach and crossing, and near-plane ordering, plus previous optics tests. Headset validation and world depth-precision assessment remain pending. No game launch.
Full Game target build succeeded; optics tests and patch whitespace/reverse-application checks passed.
