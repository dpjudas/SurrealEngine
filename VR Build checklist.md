# VR Build checklist

Everything that needs doing for a playable VR build. Engine-side only — the games' UnrealScript is off limits, so every item here has to be reachable from C++.

Ordered by implementation dependency: each phase unblocks the ones after it. Phases 1–4 are independent enough to land separately; phase 5 is the architectural risk and phase 6 is the largest build.

---

## Phase 1 — Input foundation — **DONE (not yet hardware-tested)**

Everything downstream needs new actions, new buttons, or haptics. Done in one pass over `VR/OpenXRSubsystem.cpp` and `VR/VRSubsystem.h`.

**Decision taken: VR claims the Joy keys.** `VRPlayerInput::ApplyKeybindings()` overwrites `Joy1`–`Joy16` at startup from a table in `VRPlayerInput.cpp`, called from `Engine::Run` after `OpenWindow()` (not next to `LoadKeybindings` — `IsActive()` is false until `VulkanRenderDevice` has called `InitSession`). Ini Joy bindings no longer do anything while VR is on; that is the point.

This also dissolves the enum-shift problem: bindings are regenerated from the `Button` enum every startup, so the two can't drift apart. The remaining hazard is overrun, now a build error.

- [x] Enum-shift hazard — gone, bindings are generated not inherited
- [x] Bounds check — `static_assert` in `VRPlayerInput.cpp`; **verified** by temporarily adding an 8th button and confirming the build fails with the intended message
- [x] **Grip pose** — `GripPoseAction` + `GripSpaces`, on `/input/grip/pose` for all three profiles; surfaces as `ControllerState::Grip`. Located independently of the aim pose, so a runtime tracking one and not the other doesn't silently substitute
- [x] **Trackpad** — `Button_Trackpad`, bound to `trackpad/force` on the Index (thresholded to boolean by OpenXR). Unbound on Touch/simple, where it reads as never pressed
- [x] **Haptics** — `HapticAction` + `VRSubsystem::Haptic(hand, amplitude, duration)`, defaulting to `XR_MIN_HAPTIC_DURATION` for a single tick
- [x] Wrong comment at `VRPlayerInput.cpp:118-119` — fixed

**Verified by forcing `ApplyKeybindings` on desktop** (7 buttons/hand → Joy1–14, Joy15/16 reserved):

```
before: Joy1=Fire Joy2=Jump Joy3=AltFire Joy4=Duck Joy5=NextWeapon Joy13=InventoryNext
after:  Joy1=Fire Joy2=""   Joy3=""      Joy4=""   Joy5=NextWeapon  Joy7=AltFire
        Joy8=Fire Joy12=NextWeapon Joy14=AltFire   Joy13=""  Joy15="" Joy16=""
```

Left grip is no longer Jump; left B is no longer Duck.

**Still needs a headset:** every OpenXR action added here is compile-verified only. No OpenXR runtime is reachable without the Index (SteamVR is the active runtime and needs the hardware; no Monado, so no simulator).

---

## Phase 2 — Jumping / crouching — **DONE (not yet hardware-tested)**

Move to the turn stick (up: jump, down: crouch). The Y axis is free — `UpdateTurning` only reads `.x` (`VR/VRPlayerInput.cpp:249`). Small and self-contained; depends on nothing but the phase 1 decision about whether left grip stays bound to Jump.

**Do not set `bPressedJump` directly.** The stock aliases are two commands each, and one half is game-specific:

```
Aliases[8]=(Command="Jump | Axis aUp Speed=+300.0",Alias=Jump)
Aliases[9]=(Command="Button bDuck | Axis aUp Speed=-300.0",Alias=Duck)
```

Setting the button/exec half alone skips `aUp`.

**The `SetBool`-on-`bDuck` lead is closed — routing crouch through the `Duck` alias is safe.** `bDuck` really is `var input byte` (`Pawn.uc:176`), so `UObject::SetBool`'s cast is formally UB, but it resolves to `*(uint32_t*)ptr |= 1` and sets the byte correctly without touching neighbours. Every desktop input button already relies on this. Do not fold a `SetBool` fix into this phase. Full reasoning in the research notes.

- [x] Drive the alias instead — `VRPlayerInput::UpdateJumpCrouch`, using `IK_Joy15`/`IK_Joy16` as the synthetic keys (claimed and bound to nothing, so nothing else can generate them)
- [x] Edge-trigger with a rearm gap — `JumpArmed`, gating **jump only**. A crouch is a hold, not an edge: it stays down as long as the stick is
- [x] Released on menu open too, so a crouch can't stay latched behind the pause menu

**Verified end-to-end on desktop, no headset**, by driving the exact same press/release calls from a temporary hook (since removed):

```
driving alias Duck = "Button bDuck | Axis aUp Speed=-300.0"
  after press:   buttons={bDuck=215} axes={aUp=-6000} pawn.bDuck=1 pawn.aUp=-6000
  after release: buttons={}          axes={}          pawn.bDuck=0 pawn.aUp=0
driving alias Jump = "Jump | Axis aUp Speed=+300.0"
  same tick:     axes={aUp=+6000} pawn.bPressedJump=1
  after release: axes={}          pawn.aUp=0
```

Both halves of both aliases fire, and release-by-key cleans up both. `bPressedJump` reads 1 only in the tick the alias is driven — the pawn's own tick consumes it — which is also how the desktop spacebar behaves, so one press is one jump.

---

## Phase 3 — HUD tablet — **DONE (hardware-tested)**

`RenderSubsystem::DrawGameFrame` used to run `PreRender()` / `PostRender()` once per eye, painting the console and HUD as flat 2D onto each eye at zero depth. That is why the UI "did not work at all". The fix moved the whole 2D layer onto its own draw layer — a world-space plane hung off the off-hand controller.

**Scope note held:** only `PreRender`/`PostRender` were suppressed in-eye. `RenderOverlays()` (the weapon, phase 4) was left alone, so there was no regression window.

The tablet reuses the pause-menu machinery that already existed (`DrawUICanvas`, `DrawVRMenuPlane`, the off-hand pose from `VRHands`), un-gated from `bNoDrawWorld()`:

- [x] Skip `PreRender`/`PostRender` in `DrawGameFrame` while `CurrentVREye` is set — the flat HUD no longer paints onto the eyes. The script HUD hooks still run **once** per frame via `DrawUICanvas` (previously twice, once per eye — this is strictly more correct)
- [x] Run `DrawUICanvas()` every frame in VR, not only while the menu is up
- [x] Anchor the plane to the off-hand controller (`DrawVRHudPlane`, `HandLeft`) — **fixed to the controller, not billboarded.** It rides the wrist like a watch face: normal along the hand's up axis, top edge toward the fingertips, so the player aims it by turning their wrist. Uses the live camera (like `DrawVRHands`), not the menu's frozen gaze anchor
- [x] Keep the frozen-gaze anchor for the pause menu — untouched

**Open question — transparency: RESOLVED.** The HUD canvas clears to *transparent* black (`DrawUICanvas(true)`) and the plane draws with `PF_Highlighted` — premultiplied alpha-over — so HUD pixels composite onto the world and empty areas show through. The pause menu keeps its opaque clear + opaque draw. Works because the canvas is RGBA16F and the black background makes premultiplied-vs-straight moot for the opaque HUD content.

**Element size:** the derived `uiscale` keeps desktop UI a constant apparent size across resolutions, which left HUD elements too small on a wrist panel. The HUD canvas (only, not the menu) boosts `uiscale` by a fixed amount (`VRHudUiScaleBoost`, currently +3) so elements take a larger share of the tablet. Single knob in `ResetCanvas`.

**Settled dimensions (tuned in-headset):** 4:3 canvas (1920x1440, shared with the menu), square would have been `VRMenuCanvasWidth == Height`; the physical panel is ~0.18 m wide, sat ~0.18 m back up the forearm and 0.04 m off the wrist (`tabletWidth`/`backOffset`/`upOffset` in `DrawVRHudPlane`).

**Corrections to earlier assumptions (still true):**

- The UI **cannot** be split into separate parts. HUD and menus both come out of the same `console->PostRender` script call. The tablet necessarily carries both.

**Still undecided:** whether the tablet should be a selectable/dismissable item rather than always present on the forearm. Shipped as always-present.

---

## Phase 4 — Weapon in hand — **DONE (hardware-tested)**

Attach the weapon model to the main hand (launcher-configurable left/right), alt-fire on the pill touchpad. Needs the grip pose and trackpad actions from phase 1. Independent of phase 3.

**The weapon is not the flat-2D problem.** `RenderSubsystem::DrawActor` (`Render/RenderCanvas.cpp`) renders into `MainFrame`, and `RenderScene.cpp:32-36` gives `MainFrame` the VR eye's projection when `CurrentVREye` is set. The weapon is already correctly 3D-projected with stereo depth. Its problem is placement: the script's `CalcDrawOffset` positions it relative to the camera, which welds it rigidly to the head.

So the fix is placement only, and it is local:

- [x] Override the weapon actor's `Location`/`Rotation`/`DrawScale` from the hand pose in `DrawActor`, while `CurrentVREye` is set (`RenderCanvas.cpp:152-235`). Saved and restored around the draw so nothing downstream — the desktop, the next frame, the game's own script — sees the overridden transform.
- [x] Launcher setting for main hand — `LauncherSettings::VR.WeaponHand`, read via `VRPlayerInput::WeaponHandIndex()`
- [x] Alt-fire on the trackpad — default `VR.ButtonCommands` binds `Trackpad → AltFire`; both trigger and trackpad are honoured **only on the weapon hand** (`VRPlayerInput.cpp:277-282`), so the off hand's trigger/trackpad don't also shoot
- [x] Haptic pulse on fire — `VRPlayerInput::UpdateFireHaptics()` watches the weapon's `FlashCount` (by weapon identity, so a weapon switch isn't read as a shot) and buzzes the weapon hand on each discharge, including held-trigger auto-fire

**Corrections / refinements made in-headset (still true):**

- **Align to the aim pose, not the grip pose.** The grip pose runs along the controller's handle, tilted off the direction the player points; a gun laid on it points where the physical controller lies, not where the hand model's pointer stub aims — off by the grip's rake angle. The gun now uses the same aim ray the drawn hand ball's forward stub and the menu laser use, so the muzzle lines up with the pointer line the player sees.
- **First-person meshes are modelled tiny.** They only read at the right size on the desktop because `CalcDrawOffset` parks them centimetres from the camera. Held at arm's length in VR they shrink to a toy, so the mesh is scaled up (`VR.WeaponScalePercent`, default 500) to a plausible held size.
- **Six fine-tuning knobs** (`Weapon{Forward,Right,Up}OffsetCm`, `Weapon{Pitch,Yaw,Roll}OffsetDegrees`) place the mesh in the aim pose's own frame, since each weapon mesh has its own origin and forward axis. All default 0 == gun rigidly on the raw aim pose.

---

## Phase 5 — Aim direction — **DONE (plumbing verified; feel not yet hardware-tested)**

The largest unknown in this document. Built after phase 4 so you can see the gun you are aiming. Full design in `VR Build phase 5 plan.md`; verified script interactions in the `vr_phase5_aim_split` memory.

**The framing correction.** "Aim and view are the same variable" was the *desktop* reading. In VR the view is already HMD-driven and independent of `ViewRotation`, so only **aim** needed redirecting. Implemented as Option 1 from the plan — a two-write split:

- [x] **Write A** — `VRPlayerInput::UpdateAim()` (called from `Tick`, gameplay + weapon-in-hand + hand-tracked only): capture the body anchor yaw = `Rotator(0, ViewRotation.Yaw, 0)`, map the weapon hand's aim-pose forward through that anchor into world space, write it to `ViewRotation` (roll zeroed) so script `AdjustAim` sends the shot down the hand's ray. Forces `pawn->bLook() = 1` for the tick so `PlayerWalking.PlayerMove`'s stair-look/center-view block can't drag the aim pitch.
- [x] **Write B** — `VRPlayerInput::OverrideViewAfterCalcView()` (called from `Engine::Run` right after `PlayerCalcView`, before `vrHands->Tick`/render): restore `CameraRotation`, `ViewRotation` and body `Rotation` to the anchor, so the world stays put and next frame's on-foot movement (which orients by body `Rotation`) still walks the body's way, not the gun's.
- [x] The anchor yaw rides in `ViewRotation.Yaw` between frames — snap/smooth turn advances it, Write B restores it — so aim and view rotate together on a turn.

**Verified without seeing through the headset** (SE_VRAIM_DEBUG stderr dump, since removed): over a live VR session the anchor yaw stayed pinned (constant `58424`) while the aim yaw/pitch swept the full range of the moving controller — i.e. the shot follows the hand and the view does not. No crash.

**Origin policy — FIXED (was "accept", changed after in-headset test).** The chest→hand gap measured ~30 UU (45–66 cm), too large: shots flew parallel to the aim but offset from the barrel, reading as "projectiles don't follow the pistol." `UpdateAim` now sets `weapon->FireOffset()` each frame so the script's `Start = Owner.Location + CalcDrawOffset() + FireOffset·axes` lands on the hand muzzle (`CalcDrawOffset()` reproduced in C++). Verified: predicted spawn lands on the hand with <0.2 UU residual. Applies to hitscan too. Edge case not handled: spawning at the hand when it's near a wall could clip.

**Auto-aim — DISABLED.** `UpdateAim` forces `MyAutoAim = 1.0`, so `AdjustAim` returns `ViewRotation` with no target snapping — shots go exactly where the hand points.

- [x] Decide how much of the origin mismatch is acceptable **before** starting the view/aim split — decided none; closed it via `FireOffset`.

**Still needs the headset (feel):** whether hand aim + hand-origin now feels right end-to-end, the practical pitch ceiling, and swimming/flying (those states orient movement by `ViewRotation`, so they currently move toward the aimed hand) — not yet addressed. `MovementDirectionHand` was temporarily set to Right for testing (backup at `~/.config/SurrealEngine/Settings.json.phase5bak`).

---

## Phase 6 — Weapon wheel / item wheel — **DONE (6a and 6b both hardware-tested; minor known issues remain, deferred)**

Half-Life: Alyx style. Hold A on the weapon hand, weapons arrange in a circle around the controller, move the hand to one and release to select. Same for items on the other hand. Active item shown in the item hand, activated with that hand's trigger. Needs grip pose and haptics from phase 1, and reuses the 3D actor drawing from phase 4. Full design in `VR Build phase 6 plan.md`.

New `VRWheel` class (`VR/VRWheel.h/.cpp`), a peer of `VRHands` owned by `Engine` as `engine->vrWheel`: driven from `VRPlayerInput` (open/close on A, per-frame rebuild + highlight), read by `RenderSubsystem` (draw only).

- [x] **Risk closed:** the `InventoryGroup` ambiguity doesn't apply — the wheel holds the exact `UWeapon*`/`UInventory*` object and sets `PendingWeapon`/`SelectedItem` directly (mirroring the UT99 form of `PlayerPawn.SwitchWeapon`'s tail, decompile-verified against both Unreal Gold and UT99 GOTY), never walking `Inventory.WeaponChange(byte)`'s by-group lookup at all.
- [x] Haptic tick on open and as the hand crosses each wheel entry (`VRWheel::UpdateHighlight`)
- [x] **6a — weapon wheel:** open/close on A (weapon hand → weapons, off hand → items; opening one force-closes the other), hand-displacement highlight selection, commit via `PendingWeapon`/`PutDown`/`ChangedWeapon`, cancel on centred release, fire suppressed on the wheel's hand while open, `UpdateAim`/`UpdateFireHaptics` skipped while any wheel is open, force-closes on menu open, `Reset()` wired in next to `vrHands->Reset()` on map change
- [x] **6b — item wheel + active item:** item wheel filters on `bActivatable` (decompile-verified as the predicate `Inventory.SelectNext`/`Activate` themselves use, not `bDisplayableInv`); `SelectedItem` set directly on commit; current `SelectedItem` rides the off hand the phase-4 way (`RenderSubsystem::DrawVRActiveItem`); off-hand trigger (previously always suppressed) now calls `CallEvent(SelectedItem, "Activate")`
- [x] Render: `RenderSubsystem::DrawVRWheel` — both weapon and item entries draw as 3D `PickupViewMesh` meshes (items started as camera-facing icon billboards; switched to meshes by request once sprites read as "too flat" in-headset), icon billboard only as a fallback for an item with no pickup mesh at all, plus a highlight ring/centre-cancel marker (`Draw3DLine`)
- [x] Launcher knobs added: `WheelRadiusCm`/`WheelSelectDeadzoneCm`, `WheelEntryScalePercent` (weapons)/`WheelItemScalePercent` (items)/`WheelIconScalePercent` (icon fallback) — three separate scale knobs, not one shared (see corrections below) — plus `Item*` offset/scale settings for the off-hand active item. All wired into both `VRSettingsPage` and `LauncherSettings`' JSON load/save (the latter was missed on the first pass and silently didn't persist; fixed same session)

**Corrections made in-headset (weapon wheel, 6a) — three rounds of testing on `DM-Deck16][`, UT99 GOTY:**

- **Wheel-plane orientation was wrong on the first pass** — a real coordinate-space bug, not a tuning call: `VRWheel::BuildPlaneBasis` computed the disc's facing as `vr->GetHead().Position - Center`, but `HeadPose::Position` is documented as relative to the play-space anchor while `Center` (a `VRHands` grip position) is already world space — subtracting one from the other pointed the disc in a near-arbitrary direction. Fixed to use `engine->CameraLocation`, the same "player's eye in world space" reference `VRHands`/`RenderSubsystem` already use elsewhere. Confirmed correct (perpendicular to the player) after the fix.
- **Wrong mesh for wheel entries.** Started on `PlayerViewMesh` (phase 4's held-weapon mesh) — wrong choice away from the camera: that mesh is modelled tiny and only reads right welded to the camera at `WeaponScalePercent`-style inflation. Switched to `PickupViewMesh` (how the weapon looks lying in the world).
- **Weapon orientation took two more passes.** First attempt pointed the barrel straight at/away from the camera (foreshortened to near-nothing). Second attempt pointed "up" at the camera to face the viewer, which instead rolled the model's underside into view. Settled on: barrel along the wheel's own horizontal (`PlaneRight`, perpendicular to the view direction) with genuine vertical as up (`PlaneUp`) — a side profile, confirmed as the correct read.
- **Scale needed repeated reduction:** 500% (unadjusted `WeaponScalePercent`-style default) → 250% (halved) → 60% → **6%** (current default) — `PickupViewMesh` is modelled far larger than a real-world weapon, so none of the held-weapon scale logic transfers; settled empirically after four in-headset passes.
- Unrelated finding while testing: a reproducible crash in `VulkanRenderDevice::~VulkanRenderDevice()` (AMD `vk_object_free`) whenever `Engine::Run()` returns cleanly — no `VRWheel` symbols in the trace, so unrelated to this phase. Likely the same defect as the standing Bugtracker item ("loaded game crashes after a few seconds"), or a close relative of it. Not investigated further; out of scope here.

**Corrections made in-headset (item wheel, 6b) — four rounds of testing on Unreal Gold's campaign start (Vortex Rikers → `NyLeve`'s Falls, the Translator pickup):**

- **Icon billboards read as "too small" and, separately, "too flat"** on first test. The immediate cause of "too small" was a scale-coupling bug: item icons were sized off `WheelEntryScalePercent`, the *weapon* mesh knob — when that got tuned down to 6% chasing `PickupViewMesh`'s oversized weapon models, it silently collapsed the icons to near-invisible too. Split into its own `WheelIconScalePercent` (default 36%, ~6x the collapsed size) as an interim fix.
- **Switched items from icon billboards to 3D `PickupViewMesh` entries by request** (every `Inventory`-derived pickup carries one, the same as weapons) — `WheelIconScalePercent`/`DrawWheelItemIcon` is now only a fallback for the rare item with no pickup mesh at all.
- **Item mesh scale needed its own knob too**, not the weapon one: `WheelItemScalePercent`, default 15% (2.5x `WheelEntryScalePercent`'s 6%) — a non-weapon pickup mesh doesn't necessarily model at the same raw scale a weapon's does, confirmed once items were visibly still too small at the shared 6%.
- **Item orientation is deliberately the opposite of weapons': top view, not side view.** Weapons use `(PlaneRight, PlaneUp)` (barrel horizontal, side profile); items use `(PlaneUp, -PlaneNormal)` (the item's top face toward the camera) — by request, contrasting with the weapon side-on read.
- **Off-hand active item (`DrawVRActiveItem`) now also prefers a 3D model**: tries `PlayerViewMesh` first (rare for a non-weapon item), then `PickupViewMesh` (reusing `WheelItemScalePercent`), and only falls back to the icon billboard if an item has neither.
- **Separately investigated, confirmed unrelated to the wheel:** a selected item (the Translator) can vanish from the inventory chain entirely across a level transition — not just lose "active" status. Traced as far as: the engine's travel-payload walk (`ObjectTravelInfo.cpp`) is class-agnostic and doesn't special-case any item, so no C++-level cause was found; the remaining candidates (original UnrealScript travel-accept behaviour, or a pickup/travel-snapshot timing race) are outside what static reading of this engine's C++ layer can resolve. Logged for a dedicated investigation later, not a wheel defect.
- **Minor known issues remain** from the latest pass, not yet triaged in detail — noted for a follow-up session rather than blocking this one.

**Hardware-tested and confirmed working end-to-end:** both wheels open/highlight/commit/cancel correctly, weapon switching and item selection both verified, active item rides the off hand and activates on its trigger, haptics fire on open/crossing.

---

## Phase 7 — Touch to pick up items

Today an item is only collected by walking the pawn over it — the pickup is driven by the pawn's own collision cylinder in script (`Touch`/`Bump`). In VR you should also be able to reach out and grab a floating item by touching it with a controller, the way you touch a mover or trigger already does (see the hand colliders from commit `e7f7b09d`).

The hand-collider path that fires movers and triggers is the model: it already overlaps world actors against the controller pose each frame. Item pickup is the same query pointed at `Inventory` actors — when a hand overlaps one, drive the same collection the walk-over path uses.

**Reachable from C++?** Pickup runs through the item's script `Touch`, so the engine-side move is to feed the hand as the toucher rather than to reimplement collection. Needs checking against the mover/trigger collider: whether that path can deliver a `Touch` to an arbitrary overlapped actor, or only to the ones it currently handles.

- [ ] Extend the hand collider (from `e7f7b09d`) to also test against `Inventory` actors, not just movers/triggers
- [ ] On overlap, drive the item's collection the same way walking over it does (its script `Touch`), rather than reimplementing pickup
- [ ] Haptic tick on a successful grab (reuse phase 1 `Haptic`)
- [ ] Decide which hand(s) can grab — either, or only the off-hand so it doesn't conflict with the weapon hand
