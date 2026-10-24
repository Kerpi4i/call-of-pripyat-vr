# Map facing/aspect, closing-menu presentation and PDA corners, alpha 1w

User confirmed opening-menu flash fixed; closing-menu flash remains. Map is on palm side, flattened, opaque square visible.

- Correct right map to +anatomical Y (actual dorsal side of the stock hand skeleton); preserve calibrated weapon/support wrist placement.
- Compensate native minimap widescreen correction by cropping empty horizontal capture margins: cropX=128*(1-kx). At 16:9, use UI canvas x32..224 instead of x0..256 on the square world quad. Native circular radar therefore remains circular.
- Minimap pixel shader preserves transparency outside the captured native contour, promoting covered pixels to opaque instead of forcing the whole canvas alpha to one.
- Limit RenderEnd's direct spatial-panel submission to the explicit IGame_Persistent::LoadDraw scope. Unrelated mono render passes cannot submit a flat world/body image. DoRender retains regular stereo and UI-mode transition checks.
- Suppress original local actor visual for both current view and current entity during handoff; tracked hands/items retain render/shadows.
- Native PDA frame uses a shader alpha mask with rounded outer corners (24 atlas pixels). Original texture and UVs remain, contents and hit areas unchanged. Verified ui_actor_pda atlas 1024x1024 and frame rectangle (0,256,1024,768).
- Archive inspection utility now skips zero-size directory entries.

Validation: Game target build succeeds; both pixel shaders compile SM5. Engine diff/patch checks. Headset verification still needed for facing, shape, menu closing flash and PDA appearance. No game launch.
