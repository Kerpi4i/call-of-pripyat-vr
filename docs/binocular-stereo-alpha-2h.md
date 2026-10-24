# Binocular stereo and collimation, alpha 2h

Replaces shared mono optics and fixed screen UV with two parallel optical scene cameras and angular sampling through each physical aperture. Optical cameras use the runtime eye-position baseline transformed into world space, retaining the common binocular direction without toe-in. Left/right render targets are separate and selected by the current headset render eye, including readiness checks per eye.

The aperture is a tessellated disk. Its texture coordinates are derived from the current eye-to-aperture ray in the binocular basis, with a 60 degree apparent field and the existing 15 degree scene camera. Equal viewing directions from separated eyes consequently address equal image coordinates for infinity, instead of forcing convergence on a near flat picture. The real aperture remains at model depth; its edge and eye-box darkening remain. Finite-distance scene depth comes from the two optical cameras.

Limitations: two additional full-resolution scene renders while using binoculars, approximately 4x angular zoom, fixed magnification, no optical distortion simulation. Headset fusion and model alignment require user verification; this document does not claim visual confirmation.

Validation: shader compiled with FXC ps_5_0; existing activation/eye-box tests passed; numerical checks confirmed equal angular UV for separated eyes with forward and oblique parallel rays. No game launch.
Full Game target build succeeded (xrEngine, xrGame and xrRender_R4); patch whitespace and reverse-application checks passed.
