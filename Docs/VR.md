# VR

SurrealEngine can play Unreal Gold and UT99 in VR through OpenXR. Developed and hardware-verified on a
Valve Index under SteamVR; other headsets should work through the OpenXR interaction profiles listed
below but are untested.

VR is compiled in by default (`ENABLE_OPENXR=ON`, requires Python3 at build time, defines `USE_OPENXR`,
links `openxr_loader`) and is **opt-in at runtime** via the launcher's VR tab. If VR is disabled, built
out, or no OpenXR runtime is reachable, `VRSubsystem::Create` returns a `NullVRSubsystem` whose virtuals
are all inert — VR failing to initialize never stops the desktop game from starting.

## Code layout

Everything lives in `SurrealEngine/VR/`:

| File | Role |
| --- | --- |
| `VRSubsystem.{h,cpp}` | Abstract base + `NullVRSubsystem`. Callers only ever branch on `IsActive()`. Owns the `Button` enum, `HandCount`/`ButtonCount`, `EyeView`, `HeadPose`, `ControllerState`. |
| `OpenXRSubsystem.{h,cpp}` | The real implementation. Participates in Vulkan instance/device creation (`GetRequiredVulkan*Extensions`, `GetRequiredPhysicalDevice`) before rendering; exposes per-eye poses already converted into engine axes/units. |
| `VRPlayerInput.{h,cpp}` | Locomotion, turning, button dispatch, aim, fire haptics, keybinding generation. |
| `VRHands.{h,cpp}` | Controller poses as world-space hand colliders; classifies and dispatches touches (movers/triggers/pickups). |
| `VRWheel.{h,cpp}` | The weapon/item wheel state machine. Peer of `VRHands`, owned by `Engine` as `engine->vrWheel`; driven from `VRPlayerInput`, drawn by `RenderSubsystem`. |

Rendering hooks live in `Render/` (`RenderCanvas.cpp`'s `DrawActor` override, `DrawVRHudPlane`,
`DrawVRWheel`, `DrawVRActiveItem`, `DrawVRCrosshair`, `DrawVRAimLaser`, `VisibleFrame::HeadLocalToWorld`).
Settings live in `LauncherSettings.h`'s `VR` struct and `UI/Launcher/VRSettingsPage.{h,cpp}`.

**Units.** 1 Unreal unit = 1.905 cm (`MetersToUnrealUnits = 52.4934` in `VRSubsystem.h`). Anything crossing
between Unreal units and real-world metres — poses, world-anchored UI, the cm-denominated settings — must
convert through it.

## Controls

VR **claims the Joy keys**: `VRPlayerInput::ApplyKeybindings()` overwrites `Joy1`–`Joy16` at startup from
`LauncherSettings::VR.ButtonCommands`, so ini Joy bindings do nothing while VR is on. Bindings are
regenerated from the `Button` enum every startup, so the two can't drift; overrun is a `static_assert`.
`ApplyKeybindings` is called from `Engine::Run` after `OpenWindow()`, not next to `LoadKeybindings` —
`IsActive()` is false until `VulkanRenderDevice` has called `InitSession`.

Buttons, in `Button` enum order: `Trigger, Grip, A, B, ThumbstickClick, Menu, Trackpad`. Each is a
configurable slot taking any console/exec command or ini alias, except where noted below.

Fixed (not `ButtonCommands` slots):

- **Movement stick** — on `MovementHand`; direction measured against the controller or the head
  (`MovementReference`), pointed by `MovementDirectionHand`.
- **Turn stick** — the other hand's X axis (snap / smooth / off). Its Y axis is **jump (up) / crouch (down)**.
- **A** — hold to open that hand's wheel: weapons on `WeaponHand`, items on the other. Hardcoded in `VRWheel`.
- **B on `MenuPointerHand`** — pause menu; that hand also aims the menu laser.
- **Trigger and Trackpad are honoured only on `WeaponHand`** (fire / alt-fire), so the off hand's trigger
  doesn't also shoot. The off-hand trigger instead activates the current `SelectedItem`.

Interaction profiles: Valve Index, Oculus Touch, and the OpenXR simple controller. Trackpad is bound to
`trackpad/force` on the Index only; it reads as never pressed on the other two.

## Features

**Stereo rendering + head tracking.** Per-eye views from OpenXR, rendered at `RenderScale` percent of the
runtime's recommended resolution (default 60, deliberately below 100).

**HUD wrist tablet.** The game's flat 2D layer (HUD *and* menus — both come out of the same
`console->PostRender` script call and cannot be split) is painted to a world-space plane riding the
`HudHand` wrist like a watch face, fixed to the controller rather than billboarded. `PreRender`/`PostRender`
are skipped per-eye; `DrawUICanvas()` runs once per frame. Transparent (`PF_Highlighted`, premultiplied
alpha-over) so the world shows through; the pause menu keeps its opaque clear and its frozen-gaze anchor.

**Weapon in hand.** `RenderSubsystem::DrawActor` overrides the weapon actor's `Location`/`Rotation`/
`DrawScale` from the hand pose while an eye is being drawn, then restores them, so nothing downstream sees
the override. Aligned to the **aim** pose, not the grip pose (the grip runs along the controller handle and
points off by its rake angle). First-person meshes are modelled tiny — `WeaponScalePercent` defaults to 500.
Haptic pulse on each discharge, watched off the weapon's `FlashCount` by weapon identity.

**Hand aiming (the aim/view split).** In VR the view is already HMD-driven and independent of
`ViewRotation`, so only *aim* is redirected, via two writes per frame:

- `VRPlayerInput::UpdateAim()` — maps the weapon hand's aim pose through the body anchor yaw
  (`Rotator(0, ViewRotation.Yaw, 0)`) into world space and writes it to `ViewRotation`, so script
  `AdjustAim` sends the shot down the hand's ray. Forces `bLook()` for the tick so `PlayerWalking`'s
  stair-look block can't drag the aim pitch, and forces `MyAutoAim = 1.0` (auto-aim off — shots go exactly
  where the hand points). Also sets the weapon's `FireOffset` each frame so the shot *originates* at the
  hand muzzle rather than the pawn's chest.
- `VRPlayerInput::OverrideViewAfterCalcView()` — called from `Engine::Run` right after `PlayerCalcView`,
  restores `CameraRotation`, `ViewRotation` and body `Rotation` to the anchor, so the world stays put and
  movement still orients by the *body*, not the gun.

The anchor yaw rides in `ViewRotation.Yaw` between frames, so snap/smooth turn rotates aim and view together.

**Weapon / item wheel** (Half-Life: Alyx style). Hold A to fan that hand's inventory into a circle centred
on the controller, move the hand to an entry, release to select; release centred to cancel. Haptic tick on
open and on each crossing. Commits by setting `PendingWeapon`/`SelectedItem` on the exact object — never
through `Inventory.WeaponChange`'s by-group lookup. Items filter on `bActivatable`. Entries draw as 3D
`PickupViewMesh` models (weapons side-on, items top-down), with an icon billboard only as a fallback for an
item with no pickup mesh. The selected item rides the off hand and activates on its trigger.

**Touch to pick up.** The hand collider that already fires movers and triggers also classifies unowned
`Inventory` actors (`VRHands::Classify` → `HandTarget::{None,Trigger,Pickup}`) and dispatches their script
`Touch`/`UnTouch` — the same path walking over an item uses. Guarded by a pawn→actor world trace (script
`ValidTouch` does no trace of its own), gated off while that hand's wheel is open, and configurable per
hand via `PickupHands`.

**Aim indicator.** Optional crosshair at the impact point and/or a transparent laser from muzzle to impact,
both off by default and independently toggleable. The ray is published by `UpdateAim` and traced **once per
frame** (not per eye) via `UActor::Trace(bTraceActors=true)` — the same native body script `TraceShot` goes
through, so the indicator traces the shot's own trace. Max range `AimTraceRangeUU = 10000`. Built from
`Draw3DLine`; constant apparent size.

Note that `Draw3DLine` in both the Vulkan and D3D11 backends premultiplies colour by alpha (it used to
hard-code alpha to 1.0) — that is what makes the laser transparent.

**Billboarding.** Anything facing the player must use `VisibleFrame::HeadLocalToWorld()`, which combines
`ViewRotation` with the per-eye `HeadCoords`. `ViewRotation` alone is only the body-anchor yaw (see the aim
split above) and never includes the headset's actual pose, so billboarding off it alone makes sprites face
wherever the body points. No-op on desktop, where `HeadCoords` is identity.

## Settings

All under `VR` in `~/.config/SurrealEngine/Settings.json`, edited from the launcher's VR tab. See the
inline comments in `LauncherSettings.h` for defaults and rationale; the groups are:

- **Session** — `Enabled`, `RenderScale`.
- **Locomotion** — `MovementReference`, `MovementHand`, `MovementDirectionHand`, `TurnMode`,
  `SnapTurnDegrees`, `SmoothTurnDegreesPerSecond`, `StickDeadzone`, `RoomScaleMovement`.
- **Hands** — `WeaponHand`, `HudHand`, `MenuPointerHand`, `HandColliderRadius`, `PickupHands`.
- **Weapon placement** — `WeaponScalePercent`, `Weapon{Forward,Right,Up}OffsetCm`,
  `Weapon{Pitch,Yaw,Roll}OffsetDegrees` (all offsets default 0 == weapon rigidly on the raw aim pose).
- **Active item placement** — `Item*`, the same six knobs plus `ItemScalePercent`, kept separate because
  item and weapon meshes are unrelated sizes.
- **Wheel** — `WheelRadiusCm`, `WheelSelectDeadzoneCm`, and three independent scale knobs:
  `WheelEntryScalePercent` (weapons), `WheelItemScalePercent` (items), `WheelIconScalePercent` (icon
  fallback). They are deliberately not one shared setting — pickup meshes and fixed-cm billboards have
  unrelated size logic.
- **HUD tablet** — `HudTabletWidthCm`, `HudTabletForearmOffsetCm`, `HudTabletWristOffsetCm`.
- **Aim indicator** — `Crosshair`, `CrosshairSizePercent`, `AimLaser`.
- **Bindings** — `ButtonCommands[hand][button]`.

## Testing

There is no unit-test suite; VR changes are verified by running against a real UE1 install with the headset
attached. Automated plumbing checks (does a session come up, does it survive N seconds) run headset-attached
under `gdb` and confirm a *real* session from `xrEndFrame` / `XR_SESSION_STATE_*` in the log rather than a
silent desktop fallback. Anything the user must physically do in-headset needs a **240-second timeout**.

There is no headset-less simulator (SteamVR is the active runtime and needs the hardware; no Monado), so
OpenXR action changes cannot be exercised without the Index.

## Known limitations

- **Swimming and flying** orient movement by `ViewRotation`, so in those states the player moves toward the
  aimed hand rather than the body. Not addressed.
- **Arcing projectiles** (Bio Rifle, grenades) read as wrong at range — the aim indicator draws a straight
  line, while `Grenade`/`UT_BioGel`/`BioGlob` add Z-velocity and fall under gravity. Rockets are fine (they
  fly straight at constant velocity).
- **Firing with the hand against a wall** can spawn the projectile clipped, since `FireOffset` puts the shot
  origin at the hand.
- The HUD tablet is always present on the forearm; it is not selectable or dismissable.
- Charging weapons (dispersion pistol, impact hammer, rocket launcher) mishandle held-trigger charge-up.

Open defects are tracked in `Bugtracker.md`. The phase-by-phase build history — including the in-headset
tuning rounds behind each constant above — is in the git history of `VR Build checklist.md` and
`VR Build research notes.md`, removed at the end of the VR work.
