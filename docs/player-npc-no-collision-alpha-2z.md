# Disable VR player/AI body collision, alpha 2z

Revert alpha 2y's 20 cm contact-depth reduction and remove npc_contact_reduction from the runtime equipment config. Instead reject contacts when both physics objects are character movement bodies and exactly one is the actor, under -copvr. Perform rejection before InitContact/callbacks can add body/step joints.

This covers live human NPCs and mutant movement characters, all contact normals rather than only lateral contacts. Preserve NPC/NPC interactions, actor/world collisions, hit ray/skeletal shapes, blade contact/damage, normal props and ragdoll corpse shells. Existing held-object contact exclusion is unchanged. The player and live NPCs can overlap physically; this is the requested replacement rather than a smaller movement collider.

No game launch. In-game movement and close-range combat verification remain pending.
Game target build including xrPhysics succeeded; contact eligibility reviewed and engine patch checks passed.
