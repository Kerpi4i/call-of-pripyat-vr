# Native inventory panels, alpha 1t

Restore native background atlas/UVs in VR inventory, search and shared trade panels. Background statics use white tint alpha 204 (80% opacity, 20% transparency). Item icons and text retain their native opacity.

Remove both widescreen sidepanel auto_static nodes from the local actor_menu_16.xml override. The original Steam archive and extracted original DDS/PNG assets remain unchanged. Transparency is applied at rendering, avoiding recompression or alteration of shared atlas buttons/status bars.

Movement heading now treats reverse movement as the same torso axis, clamps movement target and result to +/-30 degrees from HMD yaw. Unit tests cover backward travel and angle wrapping. Sleep and head-relative audio confirmed working by user.

No game launch authorized for this revision.
