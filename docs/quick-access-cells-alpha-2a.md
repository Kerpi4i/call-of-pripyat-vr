# Restore editable quick-access cells, alpha 2a

Remove the erroneous VR Draw-time Show(false)/Enable(false) on CUIActorMenu::m_pQuickSlot. This is the native editable quick-access reference list in the inventory menu, separate from the redundant bottom-left HUD icons which remain suppressed in DrawMainIndicatorsForInventory.

Reload quick references before InitInventoryContents' onlyBagList early return as well, so bag-only loot/trade layouts rebuild the assigned item icons and counts after ClearAllLists. Native reference assignment and existing inventory items remain unchanged.

Validation: xrGame build and diff/patch checks. No game launch. Headset validation needed for icons in inventory/loot and Touch drag assignment.
