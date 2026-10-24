# Unload in loot/trade and loading-panel mode (alpha 1p)

- VrUnloadDragged accepts mmInventory, mmDeadBodySearch and mmTrade.
  Exact dragged item only; no recursive stack unload. Own weapons work in all
  three modes, including actor trade offer lists. In loot mode a weapon in the
  dead-body/container list can also be unloaded before taking it. Trader-owned
  merchandise is excluded until purchased. Native UnloadMagazine owns ammo
  routing/spawn, and UpdateItemsPlace updates weights and trade totals.
- Main VR frame selection now treats active loading renderer, nonzero precache
  or queued loading events as spatial-screen mode, ahead of death UI/world mode.
  Direct LoadDraw already used the screen path; previously normal DoRender could
  select stereo projection for the same loading screen during the transition.
- No game test launched. Frame-mode priority test passed, including existing-level
  reload/death, menu, inventory overlay and resumed gameplay cases.

Headset validation still required for unloading from both loot panes/own trade
lists and absence of brief loading-screen duplication.

Validation: Game Release x64 build succeeded; engine diff and reverse-patch checks passed.
