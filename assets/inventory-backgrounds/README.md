# Inventory background source assets

Extracted from the installed Steam game for local editing. Original archives are unchanged.

- original/ui_actor_menu.dds and .png: shared inventory/search/trade atlas, 1024x1024.
- original/ui_actor_widescreen_sidepanels.dds and .png: widescreen borders, 1024x1024.
- original/ui_actor_menu.xml: atlas rectangle mapping.

Inventory background rectangle: x341 y0 w683 h768.
Left/search panel rectangle: x683 y0 w341 h768 (overlaps the inventory rectangle).
Do not edit unrelated buttons, bars, repair panel or blockers in the atlas.

The current VR renderer replaces background statics with dark rectangles. Restore those statics only when modified textures are ready. No edited texture has been deployed yet.
