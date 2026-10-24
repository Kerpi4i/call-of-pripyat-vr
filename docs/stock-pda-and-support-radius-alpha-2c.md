# Restore stock PDA, larger support grip and 10% panel transparency, alpha 2c

- Restore original ui_actor_pda DDS byte-for-byte in the VR override. Remove the custom PDA shader assignment and black backing from CUIPdaWnd. Existing pda_16.xml keeps widescreen sidepanels removed.
- Increase model-specific foregrip acquisition radius from 7 to 11 cm. Keep the same palm socket, fresh grip requirement and release/aiming logic.
- Inventory/search/shared trade panel tint alpha 204 -> 230, approximately 90% opacity / 10% transparency. Text and item icons remain native opacity.

Validation: original atlas byte equality, absence of widescreen PDA nodes, two-hand headless tests, xrGame build and patch checks. No game launch.
