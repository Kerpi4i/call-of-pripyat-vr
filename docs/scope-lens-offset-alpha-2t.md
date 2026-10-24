# Uniform scope lens offset, alpha 2t

Measured all 15 native scope HUD models and set each optical lens 5 mm outward from the rear ocular rim toward the player (model local -Z). This supersedes earlier per-weapon recess/offset requests, including Protecta and F2000. Retain each profile's X/Y alignment, diameter, bone attachment, tube, reticle and accuracy settings. The existing near-eye safety displacement remains active at extreme proximity.

Seven detachable and eight integrated profiles cover their native weapon variants. For F2000 isolate the body-bound rear ocular ring; for G36 isolate the lower optical channel, excluding the stock and upper reflex channel. Other models use native wpn_scope vertices. Measurement/report: reports/scope-lens-depth-5mm.json. Previous config backed up in reports/scope-lens-depth-before-5mm.ltx.

Reproducible verification: python tools/calibrate-scope-lens-depth.py checks every configured Z coordinate against the extracted original OGF model. --apply sets the coordinates. All 15 profiles verified to within one micrometre of the 5 mm offset after decimal rounding.

Configuration-only change, applied on restart. No rebuild required; no game launch or headset verification performed.
