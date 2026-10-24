# Frameless PDA texture installed, alpha 1y

Source: assets/pda-textures/previews/ui_actor_pda-no-frame-preview.png (user-selected generated version).
Prepared: assets/pda-textures/edited/ui_actor_pda-no-frame.png.
Runtime override: game-vr/gamedata/textures/ui/ui_actor_pda.dds, uncompressed RGBA DDS, 1024x1024.

Resize the preview to native atlas dimensions. Preserve the original top 256 atlas rows pixel-for-pixel: the requested frame edit is confined to the main screen region at y256..1023, while unrelated buttons and control UVs remain unchanged.

PDA shader promotes screen interior alpha to opaque and retains transparency in the removed frame region. This replaces the previous whole-quad forced opaque alpha that would otherwise turn deleted areas black.

Validation: DDS reopen and dimensions, original control rows byte equality, transparent removed border and solid interior alpha samples; pixel shader SM5 compilation. No game launch. Original Steam archive and extracted original atlas remain unchanged.
