# Opaque PDA backing and native left-hand HUD, alpha 1z

PDA: draw a solid black CUIStatic under the full native PDA frame rectangle before the textured frame/content. The underlying UI capture is opaque across the panel even where the edited atlas contains alpha. Keep the selected frameless texture.

Left hand: remove all custom watch rectangles, strap/bezel, clock text, HP percentage and custom health bar. Capture the original CUIHudStatesWnd bottom-right panel (native health, stamina and weapon/ammo display) centred at the top of the hand canvas. Native update logic and original textures remain. Draw active native status/booster icons in rows below the original panel. Restore all moved windows after capture.

Place both captures using the same anatomical dorsal-palm solution; use mirrored +/-45-degree rotation and the same 9cm plane with widescreen correction. Existing right minimap remains.

Validation: xrGame build, diff/patch checks; no game launch. Headset check needed for PDA opacity, left native HUD readability and status placement.
