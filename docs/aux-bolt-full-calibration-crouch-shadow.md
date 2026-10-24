# Auxiliary bolt, full VR recenter, crouch and hand shadows

Auxiliary bolt:
- With a firearm in either native weapon slot, the free hand can take a bolt from its rear-waist tool zone. Left rear tool zone supplies a bolt instead of the blocked knife when a firearm is in the right hand; right rear tool zone supplies a bolt when the firearm is in the left.
- A small auxiliary proxy uses the existing inventory bolt, without activating it or changing the firearm's HUD/inventory lifecycle. The firearm can continue firing. Releasing grip queues the existing motion-based native networked bolt throw.
- Native CBolt::ThrowVR accepts an inventory bolt in eHidden in addition to eIdle. It still requires single-player/local ownership, a real native fake missile and fresh projectile ID. No independent second firearm/equipment lifecycle is enabled.
- Auxiliary state resets on tracking/focus loss or actor destruction. It uses existing occupied/claimed hand gates so the same hand cannot also grab a foregrip or another item.

Right stick click, standing mode:
- The bridge now recalibrates full tracking origin XYZ, yaw/forward, measured standing eye height and calibration serial, using the current head pose.
- Body heading/estimator, hand smoothing and manipulation/throw histories reset with calibration; hand filters also reset when the new serial arrives.
- Camera base eye height and standing world-collision box derive from the measured floor eye height (clamped 1.2..2.2 m). Physical head descent continuously resizes the box and hit scale. Native width remains unchanged.
- Calibration is rejected for a head drop beyond 18 cm or classified crouch, both in engine request and bridge API.

Physical crouch:
- Enter after a 25 cm head descent, leave above an 18 cm descent (hysteresis).
- Movement acceleration and maximum-speed input scale are halved. Native movement control derives its maximum speed from acceleration magnitude, so this also halves the actual speed cap.
- Sprint intent and active sprint flags are cleared; the sprint button is ignored while crouched. Standing again does not automatically restore sprint.

Shadow fix:
- Each completed tracked-hand update snapshots the final render palette, including collapsed obsolete sleeve bones.
- SkeletonX obtains that palette through a narrow game DLL callback immediately before GPU skinning in every pass, including point-light shadows. It uses a local palette copy rather than any bone matrices recalculated from native animations between update and draw.
- Stale tracked-hand palettes suppress that draw; software skinning gets a refreshed palette/cache. Other world/NPC/item skeletons retain their normal renderer path.

Validation: full bridge/game/R4 build; physical-posture hysteresis/calibration tests; existing weapon/throw/trigger/hand/body tests. In-headset simultaneous bolt throwing, complete recenter and transient light-shadow behavior still require user verification. Game not launched.
