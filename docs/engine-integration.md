# Integration map and remaining work

Pinned OpenXRay: c62d9d343981d50f6ed176412b85d94c61b578b5.

The standalone host in `src/openxr_d3d11.hpp` now implements loader discovery,
runtime GPU selection, D3D11 requirements, session state events, reference space,
and per-eye swapchain allocation. `src/vr_diagnostic.cpp` exercises the frame loop
and projection submission on the real Quest 2. The host supports either its own
D3D11 device or a caller-owned device via `initialize(false)`,
`graphicsRequirements()`, `initializeGraphics(engineDevice)`. The caller must
select the adapter LUID/minimum feature level returned by the runtime first.
The host validates both, retains its own COM reference, and uses the external
device for the session/swapchains. `--probe-shared-device` passed on the local
Quest 2 runtime. The engine now obtains its device through R4 exports and creates
the session on that device. A C ABI bridge copies each engine backbuffer to an
OpenXR swapchain and submits the two physical eye poses/FOVs.

`patches/openxray-vr.patch` applies to the pinned checkout. Engine simulation
remains once per frame; DoRender draws twice for a 3D scene and once for a quad
menu/intro. Loading draws that bypass DoRender are copied in RenderEnd. The
normal render pipeline still shares temporal history between eyes. A controls
snapshot now connects the right Touch to the existing HUD weapon rig, the actor's
normal fire state machine and muzzle direction; actual shot callbacks send haptics.
Independent left-hand IK, physical reload and game inventory are not yet connected.

Controls ABI v1 publishes calibrated local aim/grip positions and aim basis for
both hands. The game maps these through the actor's ordinary camera matrix (before
the per-eye camera override). Input consumes the previous display frame's snapshot
before UpdateInventoryOwner, checks a 100 ms freshness limit and inhibits trigger
edges during UI/tracking/focus loss. The animated HUD rig is moved as a rigid
assembly with its right attachment anchor at the controller grip; no independent
arm IK or weapon-specific grip calibration is implemented yet. HUD models and
particles retain the VR eye projection; ordinary near-depth HUD rendering remains,
so physical wall/weapon occlusion is still unfinished.

On Windows this engine revision builds from `src/engine.sln` using MSBuild.
`tools/prepare-engine.ps1` restores pinned submodules/NuGet dependencies and a
local Windows SDK 10.0.19041.5. `tools/build-engine.ps1` builds baseline x64 Release
components; it does not enable VR or deploy to the Steam installation. The
`tools/engine-config/` property sheets keep SDK settings outside system registry
and delay include/lib/tool overrides until after Microsoft.Cpp.props, including
LuaJIT's host tools. Building an individual vcxproj without SolutionDir is invalid
for this engine's relative path settings; use solution targets.

| Engine point | Required work |
| --- | --- |
| `src/xrEngine/device.cpp`, `CRenderDevice::RenderBegin`, `RenderEnd`, `seqRender.Process` | OpenXR frame lifecycle and per-eye scene rendering; simulation must tick once per display frame, not per eye. Review renderer caches, temporal effects, culling, shadows, UI and HUD. |
| `src/xrEngine/CameraManager.cpp` | Combine calibrated tracking origin with actor transform; asymmetric eye projections, correct near/far and handedness; head collision and room scale. |
| D3D11 renderer (`xrRender_R4`) | Bind OpenXR-required adapter, query requirements before device creation, enumerate swapchain formats, acquire/wait/render/release each image, submit projection views. |
| `src/xrGame/Actor_Weapon.cpp`, `CActor::g_fireParams` | Replace camera-based firing direction with calibrated tracked weapon muzzle for the local actor only. Also update HUD/world muzzle, effects, scopes and animation. |
| `src/xrGame/Weapon*.cpp` | Route fire events into existing weapon state machine; do not create a second damage/ammo simulation. Check ray obstruction between body and muzzle. |
| `src/xrGame/WeaponKnife.cpp` | Swept collision between consecutive tracked poses with speed limits and one hit per swing. |
| Inventory/physics/entity ownership | Validate every grab/stow against entity existence and ownership. The independent prototype mutates local state directly; production integration needs authoritative commit/rollback on rejected game actions. |
| Save/load and level transition | Stop firing and clear hand references before entities are destroyed; restore standard inventory state. |

`OpenXRInput` must be destroyed before its session/instance. Attach its action set once,
before beginning the session, together with any future locomotion/UI action sets
(the current constructor attaches only its own set). It cannot be inserted into an
already attached session. All calls must follow the host's OpenXR thread/lifecycle policy.

`Interaction` expects unique item IDs, valid slot/item ownership, normalized pose
quaternions and finite inputs. Slot positions must be updated from the calibrated
body anchor each frame. Tracking loss retains the item and stops firing; releasing
the grip after tracking returns drops it. The host must update the muzzle transform
continuously while firing: FireStart is an edge event, not a per-frame aim update.

No native DLL injection into the original x86 Steam binaries is implemented.
No claim of engine compatibility can be made from these module tests alone.

## Touch UI panels

Added independent `COPVR_POINTER_ABI 1` (32 bytes) export with focused,
fresh pointer samples. Rays intersect the actual LOCAL quad pose and dimensions;
UV coordinates map to X-Ray UI_BASE_WIDTH/HEIGHT. Right hand is preferred,
left hand is fallback. Game-side TriggerGate requires release on target/hand
changes and tracking/focus loss, and dispatches existing UI mouse actions.
Cursor dialogs now select the quad rendering path. Startup logos are skipped
by launcher `-nointro`; story cinematics are preserved. Beam mesh rendering,
Touch buttons to open/close UI, scroll actions and double-click shortcuts remain
unfinished. UI selection is represented by the standard cursor at the ray hit.

## Visible beams and post-load prompt

UI rays now submit two optional 4 mm wide OpenXR LOCAL quad strips sharing
one static 8x8 cyan swapchain. The quad local Y follows the controller ray;
its front normal faces the tracked head. A hit shortens the beam to 1 cm
before the menu panel, otherwise the beam is 2.5 m long. The bridge changes
no engine D3D pipeline bindings to fill this static texture. Host teardown
releases the extra swapchain before destroying its session.

CGamePersistent::game_loaded bypasses the desktop key sequencer for a ready
single-player level under -copvr, calls the existing update_game_loaded path,
and retains normal story-intro behavior. Desktop behavior remains unchanged.
This addresses Touch events not reaching the desktop post-load sequencer.
Beam geometry and ray tests explicitly keep assertions enabled under -O2.

## Independent skeletal arms prototype

player_hud now leaves the arm model at its camera/body transform instead of
rigidly translating it with the weapon. Each HUD section attachment anchor resolves
hand, parent forearm and grandparent upper arm; a two-bone solver targets
the corresponding Touch grip position. Wrist orientation follows tracked aim
orientation with the existing animated wrist-to-weapon basis as calibration.
The old weapon positioning/muzzle path is preserved; item slot 1 also targets
the left controller. Two-bone lengths are preserved; unreachable targets
translate the shoulder. Elbow pole hints bias down and outward.

Temporary overwrite callbacks install the solved model-space bone matrices
only for synchronous CalculateBones, then restore the prior callback state.
Finger animation continues through child bones. The baseline visibility mask
is restored before the normal animation calculation; untracked arm subtrees
are hidden. Hands can render without an attached item. Monolithic HUD models
remain outside this split-skeleton implementation. No finger tracking, new
hand meshes, collision or physical reload is claimed.

## Navigation input prototype

Touch actions now include vector2 thumbsticks and primary/secondary/stick-click
buttons per hand plus optional left menu. Independent COPVR_NAV_ABI 1 export
is 40 bytes, requires a focused rendered frame/head, and suppresses untracked
hand values. Its active state is independent from the gameplay-only hand ABI,
so UI close/back works while menus are open.

Actor::UpdateCL consumes prior-frame navigation through NavigationGate with
neutral rearm on focus/tracking/UI/pause inhibition. Native movement hold
commands feed wishful locomotion bits, A/use receive press/hold/release,
stick clicks toggle sprint/crouch and right X edges rotate camera by pi/6.
This version uses fixed-speed native directional locomotion in actor space.
UI button edges are consumed at the existing post-xrWaitFrame pointer update;
Y/menu close the current dialog or toggle main menu, X/B dispatch mapped
inventory/PDA keys through Level input. UI scrolling uses right Y and frame dt.
Both controllers must be tracked for new navigation actions. Physical reload,
weapon selection and full controller-only text entry remain separate work.

## Stereo accumulator and HMD use ray

The shared light accumulator clear guard compared dwAccumulatorClearMark to
Device.dwFrame. Stereo renders twice with the same dwFrame, so the second view
could reuse the first eye's accumulated lights. phase_scene_prepare now
invalidates that marker at each VR view start; phase_accumulator clears only on
its first invocation within that view, preserving subsequent light additions.
R4 combine also disables shared-history motion blur explicitly for -copvr.
This is a concrete cross-eye reuse fix; remaining artifacts require testing.

HUDTarget::CursorOnFrame transforms the fresh calibrated head pose through the
actor's base camera matrix before its existing material-aware ray query.
Native ActorUse continues using the game's own target resolution, range and
NPC/door/script rules. Right stick negative Y adds a one-shot holster/restore
button and positive Y adds reload, only outside menu mode. Holster uses native
inventory.Activate(NO_ACTIVE_SLOT), stops firing and remembers the active slot.

Latest stereo reconstruction correction: optimized G-buffer decompression
uses actual per-eye projection P11/P22/P31/P32, not desktop FOV/aspect.
The existing packed float4 can express off-centre projections because xy are
pixel-origin offsets and zw are independent pixel scales. SSAO uv_to_eye
must use that full scale/offset formula as well; the shader previously treated
xy as symmetric ray scales. See projection_decompression.hpp and its tests.
The prior accumulator fix did not remove the user-visible Skadovsk artifact;
this new correction still requires headset validation.

CLight_DB now uses per-view duplicate suppression under -copvr. The old
light::frame_render compared simulation dwFrame, rejecting lights in the second
newly cleared package. A view-local set is cleared at Lights.Update before the
current view's spatial-light enumeration. Non-VR behavior keeps frame_render.
Light vis_prepare conservatively treats selected VR lights as visible rather
than sharing monocular occlusion rejection; spatial/frustum/portal selection
still applies. A per-view package count log helps detect empty second-eye sets.
User confirmed the preceding projection/SSAO fix removed double images/noise;
the new per-view selection fix awaits validation of right-eye lighting.

## In-game UI over stereo scene

CopVrFrame.menu now uses 0=world, 1=flat full-frame menu/loading/intro,
2=stereo world with classic UI overlay; the struct layout/ABI stays unchanged.
Mode 2 renders two projection eyes and appends an alpha composition quad from
its own swapchain. UI pointer/beam sampling stays enabled for both panel modes,
while gameplay trigger/holster sampling remains inhibited during overlay UI.

Renderer exports capture native HUD RenderUI and UICursor draws into an RGBA
texture with transparent clear. RCache MRT/depth bindings are saved/restored.
Capture occurs on the first eye only; second-eye UI calls are skipped so fonts
and cursor are identical between eyes. The first-eye UI texture is copied once
into the overlay swapchain; the scene backbuffers contain only stereo world.
The UI quad retains its classic aspect ratio/geometry and original ray hit plane.
Renderer capture resources release before renderer/device teardown or reset.

HighQuality launcher option sets native High preset before explicit VR overrides
and requests a 2560x1440 borderless backbuffer to avoid exclusive-mode fallback
on the desktop display. The bridge reports actual render dimensions in the log.

## World hint suppression and support grip

VR suppresses UIMainIngameWnd quick-action text before Draw and in its update,
the pickup icon update, and Actor::PickupInfoDraw projected item names. This
only removes presentation; target resolution and native use/pickup remain.

SupportGrip is a release-armed hysteresis state machine. It acquires near the
right aim ray (forward 0.12..0.60 m, perpendicular <=0.16 m), remains attached
while squeeze >0.25 and controller separation is 0.08..0.85 m, and resets on
tracking/UI/weapon changes. A miss requires release before another attempt.
The shared rightHand helper receives the current weapon identity and applies
support-derived direction to both HUD weapon placement and g_fireParams.
Roll remains tied to right aim up, while the second grip determines barrel
forward. Attach feedback uses left haptics at 0.2 for 20 ms. Independent hand
IK remains tracked; there is no mesh collision or forced offhand snapping.

Supported offhand now uses a complete weapon-relative wrist pose from the
native HUD foregrip. Raw left Touch roll no longer rotates the supported
virtual wrist independently. Weapon orientation aligns the model's actual
right-to-left grip vector (including lateral/vertical offsets) to the tracked
controller vector, rather than treating that vector as barrel +Z. The primary
right grip remains fixed and the virtual left hand snaps to the rigid foregrip;
changes in controller separation do not stretch the weapon. Slot-1 items
inhibit support. Releasing grip restores independent hand tracking.

Current correction after user clarification: support aim uses the previously
validated right-to-left controller direction and right-controller roll.
The experimental two-anchor correction was removed after it caused weapon
flipping in headset testing. Only the offhand's full weapon-relative wrist pose
is locked; it is composed from the final weapon pose and released on grip-up.

## Physical interaction continuation (2026-10-22; build-only, untested in game)

The current implementation extends that direction rule to either primary hand.
`oppositeWrist` in game_vr_arms.hpp converts the authored foregrip wrist through
left/right anatomical bind-pose frames. It does not rotate the weapon to match
the authored right-to-left bone vector. Slot-1 HUD equipment follows the hand
opposite the primary; an active detector no longer forces right-handed weapons.

Module boundaries:

- game_vr_motion.hpp: discontinuity rejection and bounded world-space linear/
  angular velocity estimates.
- game_vr_world_items.hpp: registry-ID based holds, native pickup/drop requests,
  confirmed-ownership consume/equip/reload actions, UI extraction and physical
  release completion. No item count or ammo count is created by these gestures.
- game_vr_body.hpp: HMD-relative body zones, native slots and detector/torch
  gestures. No waist tracker or physical backpack model is claimed.
- game_vr_tools.hpp: tracked bolt release. CBolt prepares a bounded tracked
  launch; CMissile forwards it to the native fake projectile and native reject
  event. The projectile's world velocity does not receive character motion twice.
- game_vr_weapon_parts.hpp: charging-handle grip/pull/release recognition with
  native reload. This does not yet move the bolt mesh or add a chamber state.
- game_vr_weapon_release.hpp: deliberate primary grip release requests a native
  weapon drop, preserving its inventory identity and ammunition state.

`copvr_get_ui_controls` is an optional bridge export using the unchanged controls
ABI. Gameplay controls stay inactive in UI. Only the in-game overlay receives UI
hand samples; the main menu cannot request inventory extraction. UI extraction
only accepts the actor's own ordinary inventory cells and uses the existing
GE_OWNERSHIP_REJECT path; no trade or loot ownership bypass is added.

Ammo insertion matches actual ammo sections against the active weapon's current
ammo-type array before native reload. Per-weapon optional keys are
copvr_reload_offset, copvr_reload_radius, copvr_bolt_bone and copvr_bolt_travel.
Generic defaults require future per-model calibration. Native reload animations,
magazine capacities, ammo consumption and shotgun reload subclasses remain in
charge. Separate removable magazines and individual-cartridge handling are pending.

Before ALife prepare_objects_for_save calls ClientSend/ClientSave, physical holds
are released without a throw, frozen shells are enabled and pending VR requests
are cleared. There is no new save format. Save/load behavior still needs runtime
validation. Quest items, persistent slots and active missiles are excluded from
the generic inventory/world extraction path.

Use `tools/build-bridge.ps1 -BuildOnly` for compilation and staging without tests.
The default bridge build retains its test suite for future authorized validation.
No tests or game launch were performed during this continuation. Binoculars can
be drawn/inspected but spatial magnifying lenses are not implemented; their old
flat zoom mask is suppressed in VR.
