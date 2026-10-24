# Dorsal right-hand minimap, alpha 1v

Replace the right 14.5 cm forearm gadget with a 9 cm native minimap panel on the back of the palm, 2.5 cm above its center. Remove its procedural strap/bezel. The left status watch remains unchanged.

Reconstruct the anatomical hand frame from the same calibrated wrist used by hands-only rendering, including native weapon/tool grip and support-hand attachment. The dorsal normal is opposite the anatomical palm normal; top of the map points toward fingers. It follows the hand, not viewer/head rotation. Back-facing panels remain hidden.

Capture bounds include both native radar frame and map clipping area, centered in the 256 UI canvas, preventing clipping of the standard decorative radar border. Dedicated copvr_minimap pixel shader emits opaque alpha while retaining the stock radar colors/markers/clock. Shader SM5 compilation checked. Full xrGame build required; actual comfort, facing and size require user headset validation. No game launch.
