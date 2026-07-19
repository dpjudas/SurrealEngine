# VR Build — phase 8 plan: weapon aim indicator

Hand-aimed shooting (phase 5) sends the shot down the weapon hand's aim ray. Nothing on screen tells the
player where that ray lands. On the desktop the crosshair is fixed at screen centre and the whole view is
the aim, so "where will this hit" is free; in VR the gun points anywhere in the field of view and the
answer has to be drawn.

Two indicators, **independently toggleable, both allowed on at once** (user requirement):

- **Crosshair** — a small marker sitting on the surface the ray hits, billboarded to face the player.
- **Aiming laser** — a transparent ray drawn from the muzzle to that same impact point.

Both read from one shared per-frame aim ray + trace result. That shared piece is step 1 and the only
part with any real design in it; the two indicators are then thin consumers of it.

---

## 1. What already exists (verified this session, do not re-derive)

**The aim ray is already computed every frame, in world space, by `VRPlayerInput::UpdateAim()`
(`VR/VRPlayerInput.cpp:487`).** It is not a new quantity to invent — it is the ray phase 5 already sends
the shot down:

- **Direction**: `worldForward` = the weapon hand's aim-pose `controller.Forward`, mapped through
  `Coords::Rotation(AimAnchor)` into world space. Written to `ViewRotation` (roll zeroed) as
  `Rotator aim`, which script `AdjustAim` returns unchanged because `UpdateAim` forces
  `MyAutoAim = 1.0`.
- **Origin**: `UpdateAim` sets `weapon->FireOffset()` each frame precisely so the script's
  `Start = Owner.Location + CalcDrawOffset() + FireOffset·axes` lands on
  `engine->vrHands->GetHand(WeaponHandIndex()).Position` — the drawn muzzle. So **the shot origin is the
  weapon hand pose position**, to <0.2 UU residual (phase 5, verified).

⇒ The indicator's ray is exactly `(handPose.Position, worldForward)`. **Do not recompute it from the
controller** — recomputing invites the two drifting apart after any future change to the anchor mapping,
and then the crosshair lies about where the shot goes, which is worse than no crosshair at all.

**`UpdateAim` returns early on exactly the frames where there is nothing to aim:** no `UPlayerPawn`, no
`player->Weapon()`, or `!controller.PoseValid`. It is also only called at all when
`!menuOpenBeforeButtons && !wheelOpenBeforeButtons` (`Tick`, `VRPlayerInput.cpp:175`). Every one of those
is also a frame where the indicator must not draw — so publishing the ray *from inside `UpdateAim`* gives
the correct gating for free, with no second copy of the conditions to keep in sync. This is the same
shape as the existing `AimAnchor`/`AimAnchorValid` pair.

**Collision trace API** (`Collision/TopLevel/CollisionSystem.h`):

```cpp
CollisionHitList Trace(const vec3& from, const vec3& to, float height, float radius,
                       bool traceActors, bool traceWorld, bool visibilityOnly);
CollisionHit TraceFirstHit(const vec3& from, const vec3& to, UActor* tracingActor,
                           const vec3& extents, const TraceFlags& flags);
```

`CollisionHit` carries `Fraction`, `Normal`, `Actor`, `Node`. Phase 7's wall guard already uses the
`Trace(...)` form from `VRHands.cpp:329` — precedent for a world-only zero-extent trace:
`Trace(from, to, 0.0f, 0.0f, /*actors*/false, /*world*/true, /*visibilityOnly*/false)`.

**Line drawing** (`RenderSubsystem::Draw3DLine` → `Device->Draw3DLine`, `RenderCanvas.cpp:615`):

- The Vulkan **line pipeline already blends premultiplied alpha-over**:
  `ColorBlendAttachmentBuilder().BlendMode(VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA)`
  (`RenderPassManager.cpp:212`). Transparency is available at the pipeline level.
- **But `VulkanRenderDevice::Draw3DLine` throws the alpha away**: `vec4 color = ApplyInverseGamma(vec4(Color.x, Color.y, Color.z, 1.0f));`
  (`VulkanRenderDevice.cpp:710`) — the `1.0f` is hard-coded. The scene fragment shader does
  `outColor = darkClamp(textureTex(texCoord)) * color`, so vertex-colour alpha *would* reach the output;
  it just never gets there. **This is the one thing standing between us and a transparent laser, and it is
  a two-line fix** (see 3.3).
- `LINE_DepthCued` makes a line occlude against the world (the flag the hand ball and wheel ring use);
  `LINE_None` draws it on top of everything.

**Draw-order and scene-node conventions** (`RenderSubsystem.cpp:120-126`): inside `DrawGameFrame`, per eye,
`DrawScene()` runs first and leaves `MainFrame` set up as that eye's view, then `DrawVRHands()`,
`DrawVRWheel()`, `DrawVRActiveItem()`. `Draw3DLine` reads the scene node from *whatever was last set*, not
from the frame it is passed — so any new draw function must `Device->SetSceneNode(...)` itself.
`DrawVRWheel` shows the cheap correct pattern: `Device->SetSceneNode(&MainFrame.Frame)` at the top,
`Device->SetSceneNode(&Canvas.Frame)` at the bottom.

**Billboarding**: `DrawWheelItemIcon` lays its quad along `MainFrame.ViewRotation.YAxis` /
`.ZAxis` (`RenderSubsystem.cpp:761-763`) — the current eye's own view axes. That is the camera-facing
basis to reuse for the crosshair.

**Settings wiring pattern for a bool**, three files, follow `VR.RoomScaleMovement` exactly:
`LauncherSettings.h` field → guarded `to_boolean()` load + `JsonValue::boolean()` save
(`LauncherSettings.cpp:127,329`) → `CheckboxLabel` create/SetText/SetChecked/AddWidget/Save/reset-default
(`VRSettingsPage.cpp:51,139,240,296,413,491`). **The JSON save half was silently missed on the first pass
in phase 6 and settings didn't persist** — write both halves in the same edit.

---

## 2. Step 0 — settle two questions before writing render code

Cheap, and each one can invalidate a chunk of section 3.

**0.1 — Does the ray actually predict a projectile shot, or only hitscan?** Phase 5 established that
`Start` and direction are shared by both, but a projectile with `Physics == PHYS_Falling` (UT99 Bio
Rifle, flak chunks, Unreal's grenades) arcs, and a straight line will point somewhere the shot never
reaches. Decompile check: for each of a hitscan weapon (`Enforcer`/`Automag`), a straight projectile
(`Rocket` primary), and a falling one (`Bio` / `GrenadeGun`), read `ProjectileFire`/`TraceFire` and the
projectile class's `Physics`. **Decision to record in the checklist:** the indicator is a *straight-line*
aim indicator, so weapons whose shot arcs will be honestly wrong at range. Options if that reads badly
in-headset: (a) accept — it's still right at short range and right on the majority of weapons;
(b) shorten the drawn ray to a fixed stub for arcing weapons; (c) integrate the arc. **Recommend (a) for
this phase**, with a note in the checklist. Do not build (c) speculatively.

**0.2 — What does the game's own `TraceFire` trace *against*?** i.e. whether the indicator should stop on
pawns/actors or only on world geometry. If `TraceFire` uses the standard `Trace(...)` with actors
enabled, the indicator that matches gameplay must too (so pointing at an enemy puts the crosshair on the
enemy, not on the wall behind them). Read `Weapon.TraceFire` in both games. **Expected answer: actors
included** — in which case use `TraceFirstHit` with `TraceFlags{ pawns=true, movers=true, others=true,
world=true }` rather than the world-only form phase 7's wall guard uses.

---

## 3. Implementation

### 3.1 Publish the aim ray from `VRPlayerInput` (`VRPlayerInput.h/.cpp`)

New public accessor + private state, mirroring `AimAnchor`/`AimAnchorValid`:

```cpp
// The shot ray this frame, exactly as UpdateAim aimed it: Origin is where the script will spawn the
// projectile/trace (the weapon hand's muzzle, which is what the FireOffset write puts it on), Direction is
// the normalised world-space aim. Valid only on frames UpdateAim actually redirected aim - VR on, gameplay,
// weapon in hand, hand tracked, no menu and no wheel - which is exactly the set of frames an aim indicator
// should draw on, so the renderer needs no gate of its own beyond this flag.
struct AimRay { vec3 Origin; vec3 Direction; bool Valid = false; };
const AimRay& GetAimRay() const { return CurrentAimRay; }
```

Set it at the end of `UpdateAim`, right after the `FireOffset` block (so it uses the same
`weaponHandPose.Position` that block used, and is only set on the branch where that pose is valid), and
**clear it at the top of `Tick` next to `AimAnchorValid = false`** — same lifetime discipline, same
reason: a stale ray from an earlier frame must never be drawn off.

Watch the ordering: `Engine::Run` calls `OverrideViewAfterCalcView` after `PlayerCalcView` and *before*
`vrHands->Tick`/render, and it does **not** clear the ray (only `AimAnchorValid`). Confirm the ray
survives to render time; if `Tick`'s clear happens to run in between for any code path, move the clear.

### 3.2 Trace it once per frame, not once per eye

The trace is view-independent; running it inside a per-eye draw would do it twice and could give the two
eyes different answers on a moving target. Do it once, cache the result.

**Where.** Two candidates:

- **(A) In `VRPlayerInput::UpdateAim`**, storing the hit point alongside the ray. Simple, one place, and
  the trace then sits next to the code that owns the ray's meaning. Downside: input tick runs before the
  level ticks, so the impact point is up to one frame stale against actors that moved this frame.
- **(B) In `RenderSubsystem`, once per frame** before the eye loop.

**Recommend (A).** One-frame staleness against a moving target is imperceptible at 90 Hz and is *the same
staleness the shot itself has* (the shot is fired from the button path inside the same input tick — see
the `UpdateTurning`/`UpdateAim`-before-`UpdateButtons` ordering comment at `VRPlayerInput.cpp:161-168`).
That makes (A) not merely acceptable but strictly more truthful: the crosshair shows where a shot fired
*right now* would go. Record this reasoning; it is the sort of thing that looks like a bug later.

Extend the struct:

```cpp
struct AimRay { vec3 Origin, Direction; float Length; bool Valid = false; bool Hit = false; };
```

`Length` is the distance to the impact, or the max range when nothing was hit. Guard the trace with the
same `pawn->XLevel()` null check the phase 7 contact code uses.

**Max range** (checklist item 4): a fixed constant in Unreal units. Reference points — UT99 hitscan
`TraceFire` uses ranges on the order of 10000 UU. **Recommend a single named constant, `AimTraceRangeUU =
10000.0f`**, not a launcher setting: it is not a preference, and a wrong value is invisible to the player
except as a missing indicator. **Past max range with no hit: draw nothing** (no crosshair, and the laser
either stops at max range or is also suppressed — see 3.4). A crosshair floating in mid-air at an
arbitrary distance claims a hit point that does not exist.

### 3.3 Make `Draw3DLine` honour alpha (`VulkanRenderDevice.cpp`, and check D3D11)

Required for the laser; nothing else in the plan depends on it.

```cpp
// was: vec4 color = ApplyInverseGamma(vec4(Color.x, Color.y, Color.z, 1.0f));
vec4 color = ApplyInverseGamma(vec4(Color.x, Color.y, Color.z, 1.0f));
color = vec4(color.rgb * Color.w, Color.w);   // premultiplied: the line pipeline blends ONE / ONE_MINUS_SRC_ALPHA
```

Premultiply **after** the gamma call, and pass alpha through un-gamma'd — gamma applies to colour, not to
coverage. **Regression surface, enumerated (grep-verified this session, all `Draw3DLine` call sites):**

| Caller | Alpha | Effect of the change |
|---|---|---|
| `RenderSubsystem.cpp:443` menu laser | `1.0` literal | none |
| `RenderSubsystem.cpp:604,611` hand ball + stub | `1.0` literal | none |
| `RenderSubsystem.cpp:798,804,806,816` wheel ring/highlight | `1.0` literal | none |
| `VisibleMesh.cpp:945,960` AI route debug lines | `1.0` literal | none |
| **`NCanvas.cpp:278` `Draw3DLine_U227`** | **`Col.A / 255.0`, script-supplied** | **behaviour changes** |

The last row is the exception. `Draw3DLine_U227` is the Unreal 227 script native, and it already passes
the script's alpha through — today that alpha is silently discarded and every such line draws opaque.
After the change it is honoured, so a 227 script asking for a translucent line gets one, and one asking
for `A = 0` gets nothing where it previously got an opaque line. That is arguably the bug being fixed
rather than a regression (the script asked for alpha and was ignored), and 227 is not among the supported
versions per `Docs/Status.md` — but it is a real behaviour change and belongs in the commit message.
If it needs to be avoided, keep the old opaque path for that native by having it pass `1.0` explicitly.

Do the same in `D3D11RenderDevice::Draw3DLine` (`D3D11RenderDevice.cpp:2188`) *if* it hard-codes alpha the
same way — check its blend state first; if D3D11's line blend is not premultiplied-alpha, leave it alone
and note that the laser is Vulkan-only (the VR path is Vulkan-only anyway, since `OpenXRSubsystem`
participates in Vulkan instance/device creation).

**Fallback if this turns out not to work** (the null-texture bind's alpha is not 1, or the depth-write on
the line pipeline makes translucent lines fight each other): draw the laser as a thin translucent quad via
`DrawGouraudPolygon` with `PF_Translucent`, billboarded like `DrawWheelItemIcon`. That needs a texture,
which is the reason it is the fallback, not the first choice.

### 3.4 Draw the laser — `RenderSubsystem::DrawVRAimLaser()`

New function, called from `DrawGameFrame` alongside the other VR draws (after `DrawVRHands()`, before
`DrawVRActiveItem()`), gated on `CurrentVREye && LauncherSettings::Get().VR.AimLaser && ray.Valid`.

- `Device->SetSceneNode(&MainFrame.Frame)` at the top, `&Canvas.Frame` at the bottom (the `DrawVRWheel`
  pattern).
- A single `Draw3DLine` from `ray.Origin` to `ray.Origin + ray.Direction * ray.Length`, with
  `LINE_DepthCued` so it terminates properly against geometry even if the trace and the depth buffer
  disagree by a pixel.
- Colour: a warm red, alpha around `0.35` — the "transparent ray" the request asks for. Start there and
  expect to tune it in-headset. **Put the alpha and colour in named constants at the top of the function,
  not inline**, so a tuning pass is a one-line edit (this is what made the phase 6 scale tuning survivable).
- **Start the line slightly forward of the muzzle** (a few UU along `Direction`) so it does not draw
  inside the hand ball / weapon mesh and read as a bright dot on the gun.
- If `!ray.Hit` (nothing within max range): still draw the laser out to max range. A laser that vanishes
  when pointed at the sky is worse than one that fades into the distance — and unlike the crosshair, the
  laser is not claiming a specific impact point.

### 3.5 Draw the crosshair — `RenderSubsystem::DrawVRCrosshair()`

Gated on `CurrentVREye && LauncherSettings::Get().VR.Crosshair && ray.Valid && ray.Hit`.

**Build it out of `Draw3DLine`, not a texture.** The checklist says "crosshair sprite"; recommend
overriding that. A sprite needs a texture, and the only candidates are the games' own HUD crosshair
textures (`ChallengeHUD.Crosshairs[]` in UT99, a different arrangement in Unreal) — game-specific, version-
specific, and exactly the kind of asset lookup that makes a feature work in one game and silently not the
other. A line-drawn crosshair needs no asset at all, works identically in both games, and reuses the
`DrawWheelHighlightRing` machinery already proven in-headset.

- Billboard basis: `MainFrame.ViewRotation.YAxis` (right) and `.ZAxis` (up), the same two
  `DrawWheelItemIcon` uses. **Billboarded to the eye, not aligned to the surface normal** (checklist item
  2) — a normal-aligned crosshair collapses to a line at grazing angles, which is precisely when you most
  need to read it.
- Shape: a ring of ~16 segments plus four short ticks, or a simple cross with a centre gap. Keep it
  small.
- **Size**: constant *apparent* size, i.e. scale the radius with distance to the eye
  (`radius = k * distance`), not a constant world size. A world-constant crosshair is a dot at 50 m and
  swallows the screen at 1 m. `k` such that it subtends a couple of degrees.
- **Offset it toward the eye** by a small amount along `-Direction` (or along the hit normal) so it does
  not z-fight with the surface it is sitting on. Use `LINE_DepthCued` regardless, so a crosshair on a
  surface around a corner is occluded rather than floating.
- Colour a clearly different hue from the laser so the two read as separate elements when both are on.

### 3.6 Two independent launcher settings

Both `bool`, both default **off** — this is an aid, not the intended default presentation, and every VR
frame already has plenty going on. (Reconsider after in-headset testing; changing a default is one line.)

```cpp
// Draw a crosshair where the weapon's shot will land (see RenderSubsystem::DrawVRCrosshair). Independent of
// AimLaser: either, both or neither. Off by default - hand-aimed shooting is meant to be read off the gun,
// and this exists for the players and the weapons where that isn't enough.
bool Crosshair = false;

// Draw a transparent ray from the muzzle to the impact point (RenderSubsystem::DrawVRAimLaser).
bool AimLaser = false;
```

Wire both through all six sites listed in section 1's settings-pattern note. Checkbox text:
"Show a crosshair where the weapon will hit" / "Show an aiming laser from the weapon".

**They must not be collapsed into one tri-state dropdown**, even though it would be fewer widgets — the
requirement is explicitly that both are independent and can be on together, and a dropdown makes that
combination unrepresentable.

### 3.7 Decide when the indicator renders (checklist item 3)

The checklist leaves this open between: always / only when the weapon can fire / only while a fire button
is touched or held.

**Recommend: always, while the phase-5 aim is live** — which is precisely `ray.Valid`, and is therefore
already implemented by section 3.1 with no extra condition. Reasons: "only while the weapon can fire"
needs an ammo/melee predicate that is script-side and game-specific (the exact class of thing phases 6 and
7 kept having to decompile), and an indicator that blinks out when you run dry is a confusing way to learn
you are out of ammo. "Only while the trigger is touched" needs the Index's capacitive touch, which is not
among the actions phase 1 bound.

Note that `ray.Valid` already suppresses both indicators with the menu up, with a wheel open, with no
weapon held, and with the hand untracked — all correct, all free.

---

## 4. Decisions taken here (and why)

1. **The indicator reads the ray phase 5 already computes, it never recomputes one.** A second derivation
   of "where the shot goes" is a lie waiting to happen.
2. **Traced once per frame in the input tick, not per eye.** Both eyes agree, and the staleness matches
   the shot's own.
3. **Crosshair drawn from lines, not a game texture.** No per-game asset lookup, works in both games.
4. **Billboarded, not surface-aligned.** Readable at grazing angles.
5. **Constant apparent size, not constant world size.**
6. **Max range is a constant, not a setting**; nothing drawn for the crosshair past it, laser drawn to it.
7. **Two independent booleans, both default off.**
8. **Straight-line only** — arcing projectiles will be wrong at range, accepted for this phase (0.1).

---

## 5. Test plan

Automated first (the phase 7 pattern: `SE_AUTOLAUNCH=0 timeout 45 gdb -q -batch -ex run -ex "bt 20" -ex
quit --args ./SurrealEngine "<gameRoot>"`, `exit 124` is the success signal):

1. Build succeeds; desktop mode (`VR.Enabled=false`) unaffected — **explicitly confirm the `Draw3DLine`
   alpha change did not alter any existing line's appearance** (hand ball, wheel ring, menu laser).
2. VR session runs 45 s with both settings off, then both on, on both games — the per-frame trace executes
   against a live level with no crash.

Then in-headset (**240 s timeout, per CLAUDE.md**):

3. Crosshair only: sits on the surface, faces the player, stays readable at grazing angles and at both
   very close and very long range.
4. Laser only: visibly transparent (the world reads through it), terminates at geometry, does not start
   inside the hand/gun.
5. **Both on together** — the explicit requirement; confirm they are distinguishable and neither is drawn
   twice or fighting the other in depth.
6. Fire a shot and confirm it lands where the crosshair says, on a hitscan weapon and a straight
   projectile weapon. Then on an arcing one, to see how badly 0.1's accepted limitation reads.
7. Menu open, wheel open, no weapon held, controller set down (tracking lost) — indicators disappear in
   all four.

---

## 6. Known risks

- **The `Draw3DLine` alpha change touches a shared render path.** Mitigated by every existing caller
  passing `a = 1.0`; still, it is the one edit here that can affect the desktop game, so test 1 is not
  optional.
- **Depth write on the line pipeline** (`DepthStencilEnable(true, true, false)`) means translucent lines
  write depth and can occlude each other in draw order. With one laser and one small crosshair this is
  unlikely to show; if it does, the fix is to draw the crosshair before the laser, or a dedicated
  no-depth-write line pipeline (do not build that pre-emptively).
- **Arcing projectiles** (0.1) — accepted, recorded.
- **The trace's actor set** depends on 0.2's answer; getting it wrong makes the crosshair sit on the wall
  behind an enemy, which is the single most visible way this feature can be wrong.

---

## 7. Out of scope

- Any change to the games' UnrealScript, or to the flat HUD crosshair on the wrist tablet (phase 3).
- Arc integration for falling projectiles.
- A laser dot / world decal on the impact surface (the crosshair covers that role).
- Per-weapon indicator tuning, spread/accuracy cones, and anything that needs script-side weapon state.
