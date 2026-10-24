# Closer human NPC contact for VR melee, alpha 2y

Reduce lateral collision separation by 0.20 m only for VR actor/stalker movement-character contact pairs, before native character InitContact can create its movement/step joints. Subtract the configured inset from contact penetration; shallow contacts within the inset are ignored, deeper contacts retain their residual collision response. This allows closer approach without disabling the pair entirely.

Both physical objects must be character movement bodies, and the pair must be actor versus human stalker. Restrict to mainly horizontal normals (absolute Y <0.5). Preserve vertical contacts, world geometry, nonhuman creatures, ordinary props, NPC/NPC interactions, non-VR mode and corpse/ragdoll shells. The earlier held-object contact rejection remains first in the same dispatch block.

Do not alter actor box sizes, NPC models, skeletal collision forms, knife reach, ray picking or damage registration. Native actor supplementary NPC restrictors were already configured at 0.1 m; the main movement-body contact was the relevant remaining separation.

Configuration: [body] npc_contact_reduction=0.20 metres, clamped [0,0.30], loaded by xrPhysics at first relevant contact. Set 0 and restart to restore native separation. Physical contact distance and close-range NPC behavior require in-game verification. No game launch.
Game target build succeeded, including xrPhysics and xrGame; engine patch whitespace/reverse-application checks passed. Contact pair/normal eligibility reviewed; live knife reach unverified.
