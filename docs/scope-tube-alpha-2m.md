# Scope tube and fitted reticle, alpha 2m

Restore the seven detachable-scope aperture radii by +0.005 m (+1 cm diameter), retaining the Protecta 0.7 cm recess and other sights' 1 cm recess. Binocular dimensions and behavior are unchanged.

Replace the weak 28 percent vignette with a ray-tested black tube. The virtual tube is 3.5 cm long, with a far opening 1.20 times the entrance radius. Each eye-to-aperture ray is extended to that opening. Pixels outside it are opaque black, producing a visible directional crescent as the eye leaves the scope axis. The original collimated stereo world sampling remains unchanged.

The native PSO/SUSAT scale now uses coordinates at the far opening, independent of the wider collimated scene UV. Native reticle UV extents of +/-0.36 horizontally and +/-0.46 vertically fit the full scale including the lower range markings within the far circle. Baked glass is removed through alpha separation; native dark housing at the outer edge blends into the black tube. This is a finite-distance reticle plane for this prototype; visual comfort/alignment requires headset verification.

A dedicated scope vertex shader decodes two 16-bit far-opening coordinates from the existing vertex colour before interpolation. This avoids adding a new UI vertex format or quantizing live eye movement to 8 bits. The pixel shader clips the far aperture with a narrow smooth edge and renders black opaque tube walls. No effect is applied to binoculars.

Validation: new scope VS and PS compiled successfully with FXC SM5. Math tests cover centred opening, mirrored lateral eye movement, visible opaque rim and invalid eye relief, alongside existing binocular tests. No game launch. Headset evaluation remains pending.
xrGame build succeeded; native scale-extents checks, configured radii/recess checks and engine patch checks passed.
