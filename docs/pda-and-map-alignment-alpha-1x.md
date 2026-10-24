# PDA opacity and native minimap alignment, alpha 1x

- PDA uses original rectangular atlas without rounded mask, opaque frame output. Removed widescreen sidepanels stay removed.
- Right-hand minimap rotated 45 degrees around its surface normal.
- Native map WorkingArea follows actual m_clipFrame absolute position during VR update. Update map spots after moving the root for wrist capture; restore root and map positioning afterwards.
- Add solid black circular backing using precisely the native map's WorkingArea centre, radius and widescreen correction. Transparent canvas outside the circle stays transparent.
- HUD render requires a successful tracking pose solve for this player_hud instance. Reset readiness each update before restoring native bone visibility. This prevents render_hud from submitting the briefly restored native model when controls resume after closing a menu but before a tracked pose is available. Headset confirmation needed.
- Extract original PDA frame/buttons atlas ui_actor_pda and noise ui_pda2_noice to assets/pda-textures/original as DDS and PNG with descriptor XML. Wide-screen sidepanel source retained for reference, not rendered. No original Steam resources modified.

Validation: xrGame build succeeds; opaque PDA pixel shader compiled SM5; engine diff/patch consistency checks. No game launch.
