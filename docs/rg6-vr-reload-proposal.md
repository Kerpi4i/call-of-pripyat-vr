# RG6 VR reload proposal

Proposed interaction, not implemented:

1. An offhand grip on a dedicated latch contact opens the weapon through its authored opening articulation. Ordinary support grip stays independent.
2. The rear-waist grip+trigger chord produces one full-size VOG-25 proxy, using the established reload hand conventions.
3. Release the round at the accessible empty chamber to consume exactly one compatible inventory round. Use a forgiving single insertion contact with feedback.
4. Index the drum to the next available chamber after insertion. Maintain six occupancy states and skip live chambers. Use rounds themselves as visible state, with a brief valid-target highlight if needed.
5. Close through the dedicated opening contact. Allow partial loading and immediate firing once closed; no additional charging gesture in the initial gameplay prototype.

Keep native grenade projectile creation/ownership synchronized one-to-one with loaded rounds. Opening does not replenish ammo or discard unfired rounds. Release elsewhere cancels the inventory proxy. Hands work symmetrically. Disable fire while open. Persist chamber occupancy, active chamber and opening state. The native model has wpn_body/body/lock/reload/target/rotategrenades/grenade/trigger bones; verify authored hinge and axis before implementation. This is an ergonomic game interaction proposal, not a statement of real weapon operation.
