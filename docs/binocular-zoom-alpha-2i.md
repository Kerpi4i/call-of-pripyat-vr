# Binocular aperture and zoom, alpha 2i

Reduce lens radius from 0.028 to 0.023 m: physical diameter decreases exactly 1 cm. Remove the artificial radial dark ring and gradual eye-box brightness attenuation; an invalid/outside eye box still blocks the optical image. Stereo cameras and collimated ray mapping remain unchanged.

While viewing through the binocular, the right stick vertical axis continuously changes the magnification factor from 0.5 to 1.5, initially 1.0 (the previous magnification, exactly the arithmetic midpoint). Up increases magnification, down decreases. Camera vertical FOV is calculated through tangent scaling, not linear FOV interpolation. Both optical cameras share the factor. Changing magnification does not change headset FOV. Neutral stick arms zoom input; invalid/inactive navigation disarms it. Time steps are capped at 50 ms and duplicated timestamps cannot apply zoom twice. The selected magnification persists in this process.

Validation: binocular unit tests cover initial midpoint, increase, both bounds, inactive input and neutral arming, plus existing eye-box tests. Headset confirmation remains required. No game launch.
xrGame build succeeded; unit tests and engine patch checks passed.
