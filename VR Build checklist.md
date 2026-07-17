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

## Phase 4 — Weapon in hand

Attach the weapon model to the main hand (launcher-configurable left/right), alt-fire on the pill touchpad. Needs the grip pose and trackpad actions from phase 1. Independent of phase 3.

**The weapon is not the flat-2D problem.** `RenderSubsystem::DrawActor` (`Render/RenderCanvas.cpp:125`) renders into `MainFrame`, and `RenderScene.cpp:32-36` gives `MainFrame` the VR eye's projection when `CurrentVREye` is set. The weapon is already correctly 3D-projected with stereo depth. Its problem is placement: the script's `CalcDrawOffset` positions it relative to the camera, which welds it rigidly to the head.

So the fix is placement only, and it is local:

- [ ] Override the weapon actor's `Location`/`Rotation` from the grip pose in `DrawActor`, while `CurrentVREye` is set
- [ ] Launcher setting for main hand
- [ ] Alt-fire on `trackpad/force` or `trackpad/touch`
- [ ] Haptic pulse on fire

---

## Phase 5 — Aim direction (architectural risk — decide before building)

The largest unknown in this document. Do it after phase 4 so you can see the gun you are aiming.

Fire direction comes from `AdjustAim`, which is script and reads `ViewRotation`. `ViewRotation` is also what `PlayerCalcView` returns, which is what `Engine.cpp:192-193` assigns to `CameraRotation`. **Aim and view are the same variable** — pointing the gun with your hand today swings the camera with it.

Decoupling them means the engine owns a separate view rotation and overrides `CameraRotation` after `PlayerCalcView`. That also interacts with `UpdateTurning` writing `ViewRotation` for snap turns.

Even then: shots still *originate* at `Owner.Location + FireOffset` — the pawn's eye — because that is computed in script. A projectile leaving your chest while the muzzle is at your hand is a mismatch no engine-side hook fully closes.

- [ ] Decide how much of the origin mismatch is acceptable **before** starting the view/aim split

---

## Phase 6 — Weapon wheel / item wheel

Half-Life: Alyx style. Hold A on the weapon hand, weapons arrange in a circle around the controller, move the hand to one and release to select. Same for items on the other hand. Active item shown in the item hand, activated with that hand's trigger. Needs grip pose and haptics from phase 1, and reuses the 3D actor drawing from phase 4.

The pieces exist:

- `pawn->Inventory()` is a walkable native chain
- `Pawn.SelectedItem` and `Inventory.InventoryGroup` are both in `PropertyOffsets`
- `engine->ExecCommand` reaches the script exec functions (`SwitchWeapon`, `ActivateItem`, `NextItem`)

**Render cost is accepted.** Drawing every carried weapon's `PlayerViewMesh` each frame while the wheel is open is fine — the wheel is only up for a moment at a time, and the cost buys the thing that makes selection readable. No measurement gate on this.

- [ ] **Risk:** `SwitchWeapon` selects by `InventoryGroup`, so two weapons sharing a group are indistinguishable through that path
- [ ] Haptic tick as the hand crosses each wheel entry

---

## Phase 7 — Touch to pick up items

Today an item is only collected by walking the pawn over it — the pickup is driven by the pawn's own collision cylinder in script (`Touch`/`Bump`). In VR you should also be able to reach out and grab a floating item by touching it with a controller, the way you touch a mover or trigger already does (see the hand colliders from commit `e7f7b09d`).

The hand-collider path that fires movers and triggers is the model: it already overlaps world actors against the controller pose each frame. Item pickup is the same query pointed at `Inventory` actors — when a hand overlaps one, drive the same collection the walk-over path uses.

**Reachable from C++?** Pickup runs through the item's script `Touch`, so the engine-side move is to feed the hand as the toucher rather than to reimplement collection. Needs checking against the mover/trigger collider: whether that path can deliver a `Touch` to an arbitrary overlapped actor, or only to the ones it currently handles.

- [ ] Extend the hand collider (from `e7f7b09d`) to also test against `Inventory` actors, not just movers/triggers
- [ ] On overlap, drive the item's collection the same way walking over it does (its script `Touch`), rather than reimplementing pickup
- [ ] Haptic tick on a successful grab (reuse phase 1 `Haptic`)
- [ ] Decide which hand(s) can grab — either, or only the off-hand so it doesn't conflict with the weapon hand
