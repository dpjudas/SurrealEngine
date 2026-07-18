# VR Build — Phase 5 plan: aim direction

Planning document for the aim/view split, the item the checklist flags as *the largest
architectural unknown*. Engine-side only — the games' UnrealScript is off limits, so every
lever here has to be reachable from C++. Nothing below is built yet; this is the design pass
the checklist asks for ("decide how much of the origin mismatch is acceptable **before**
starting the view/aim split").

Session date: 2026-07-18. Branch: `vr_renderer`. Companion to `VR Build checklist.md` and
`VR Build research notes.md`.

---

## What the goal actually is

Point the weapon hand somewhere and have shots go **there** — where the drawn gun's muzzle
points (the aim pose ray the phase-4 weapon is already laid along) — without the world
swinging with the hand.

---

## Correcting the framing carried over from the desktop analysis

The research notes say *"aim and view are the same variable"* and *"pointing the gun with your
hand today swings the camera with it."* That was the **desktop** reading and it is **not** the
whole story in VR. Verified against the code this session:

- **The view is already decoupled from the head in VR.** `RenderScene.cpp:32-34` +
  `BuildVREyeView` (`RenderScene.cpp:60-88`) layer the live HMD pose (`CurrentVREye->Forward/
  Right/Up`, from OpenXR) on top of `CameraRotation`. `CameraRotation` is only the play-space
  anchor yaw. So the player already looks around freely with their head; head-look never
  touches `ViewRotation`.
- **`CameraRotation` in VR is the body/anchor yaw.** `Engine.cpp:203-210` seeds
  `CameraRotation` from `viewport->Actor()->Rotation()` and then runs `PlayerCalcView`, which
  (in script) returns `ViewRotation`. Snap/smooth turn drives it through
  `VRPlayerInput::UpdateTurning` writing `player->ViewRotation().Yaw` (`VRPlayerInput.cpp:386-390`),
  which `PlayerPawn.UpdateRotation()` treats as master and copies onto the body.
- **Aim is `ViewRotation`.** `AdjustAim` is script (no native anywhere) and reads `ViewRotation`.
  In VR `ViewRotation` currently carries only the body/anchor yaw (plus whatever pitch the game
  keeps), **not** the head pose and **not** the hand. So today shots leave along the body yaw
  at (roughly) level pitch, regardless of where the head looks or the gun points.

**So the real phase-5 problem is narrower and better-defined than the notes imply:** the view is
fine; only **aim needs to be redirected from the body yaw to the weapon hand's aim ray**, and the
one hazard is that `ViewRotation` is the *input* to both aim (good, we want to write it) and to
`CameraRotation`/the view (bad, we must not let the hand swing the world).

> Correction to log in `VR Build research notes.md` once this is built: the "aim and view are the
> same variable / hand swings the camera" statement is desktop-only; in VR the view is already
> HMD-driven and independent of `ViewRotation`.

---

## The coupling to break, precisely

`ViewRotation` feeds two consumers in the same frame:

1. **Aim** — script `AdjustAim` reads it when the trigger fires. *We want this to be the hand.*
2. **View** — `PlayerCalcView` returns it → `Engine.cpp` assigns it to `CameraRotation` →
   `BuildVREyeView` composes HMD on top. *We must keep this on the body/anchor yaw, not the hand.*

If we naively set `pawn->ViewRotation = handAim` each frame, aim becomes correct but
`CameraRotation` becomes the hand direction and the rendered world yaws/pitches with the gun hand
(and the HMD pose gets layered on top of *that*) — unplayable.

So the split is two writes, not one:

- **Write A (aim):** set `pawn->ViewRotation` to the weapon-hand aim (pitch + yaw, in engine
  units) at the right point in the frame so `AdjustAim` reads the hand.
- **Write B (view):** immediately after `PlayerCalcView`, overwrite `engine->CameraRotation` with
  the separately-tracked body/anchor yaw, so the render + HMD composition ignores the
  hand-driven `ViewRotation`.

Both levers already have precedent on the branch: Write A is the exact mechanism
`UpdateTurning` uses (`player->ViewRotation().Yaw = ...`); Write B is a one-line override at
`Engine.cpp:206-211`'s tail, and `CameraRotation` is already a plain engine-owned field the VR
menu path freezes and restores (`VRMenuFrozenCameraRotation`).

---

## Candidate approaches

### Option 1 — Hand-aim into `ViewRotation`, restore the view from a tracked anchor yaw (recommended)

Keep an engine-owned **anchor yaw** (the current `CameraRotation` role: seeded from the pawn,
advanced by snap/smooth turn, never touched by the hand). Each frame:

1. Before the level ticks, set `pawn->ViewRotation` = weapon-hand aim direction (from the same
   aim pose phase 4 lays the gun along, `VRHands::GetHand(WeaponHandIndex())`), converted to a
   `Rotator`.
2. Let the level tick and `PlayerCalcView` run — aim/fire now use the hand.
3. After `PlayerCalcView`, set `engine->CameraRotation` = anchor yaw (level pitch/roll), so the
   view stays body-yaw + HMD.

- **Pro:** reuses two mechanisms already on the branch; aim follows the hand exactly; view
  untouched. Smallest new surface.
- **Con:** `ViewRotation` now flips to the hand every frame. Anything *else* script does with
  `ViewRotation` between our write and our restore inherits the hand direction — see Risks
  (body facing, `DesiredRotation`, pitch clamp, movement heading). Has to be audited.

### Option 2 — Only redirect at fire time

Leave `ViewRotation` on the body yaw almost always; snap it to the hand aim only on the frames a
shot is actually being generated, then restore. Harder: fire is generated inside the script tick
(`AdjustAim` is called from the weapon's own `Fire`/`TraceFire`), so there is no clean C++ seam
"just before the shot" without driving fire ourselves. Likely means the engine detecting the
trigger edge and setting `ViewRotation` for that whole tick — which is Option 1 restricted to
fire frames, and buys little because the same-tick side effects still apply.

### Option 3 — Leave aim on the body, don't split at all

Accept that shots go where the body faces and the gun is decorative-aim-only. Rejected: defeats
the point of a hand-held weapon; the phase-4 muzzle/pointer alignment would be a lie.

**Recommendation: Option 1**, with the same-tick side effects of `ViewRotation` audited and
neutralised one by one (below). Fall back to Option 2's fire-frame scoping only if a side effect
turns out to be un-neutralisable.

---

## The origin mismatch — the decision the checklist demands up front

Even with direction fixed, shots still **originate** at `Owner.Location + FireOffset` — the
pawn's eye/chest — because that is computed in script (`AdjustAim`/`TraceFire`/projectile spawn),
unreachable from C++ without moving the pawn. The muzzle is at the hand; the bullet is born at
the chest. Consequences to weigh **before** coding:

- **Hitscan** (most UT/Unreal weapons): the trace starts at the chest and goes in the hand
  direction. At any range past a metre or two the parallax is negligible and it feels right.
  Up close (an enemy beside a doorframe you're peeking past) the shot can clip the frame the
  muzzle clears, or hit something the muzzle doesn't point at.
- **Projectiles** (rockets, flak, biorifle): spawn visibly at the chest and fly off — a rocket
  appearing from the sternum rather than the launcher is the most noticeable case.
- **Muzzle flash / smoke**: spawned in script at the chest offset too; will not sit on the drawn
  muzzle.

Options for the origin, cheapest first:
1. **Accept it** (chest origin, hand direction). Zero engine work beyond the direction split.
   Probably fine for hitscan-dominant play; jarring for rockets.
2. **Move the pawn's `FireOffset`** per weapon so the spawn sits nearer the hand. `FireOffset` is
   a script property; reachable to *write* from C++? — **needs checking** (is it in
   `PropertyOffsets`, is it per-weapon, does the script overwrite it each fire). Even if writable,
   it is a fixed offset in the pawn's frame, not the live hand position, so it only approximates.
3. **Nudge the pawn `Location` for the fire tick** so `Owner.Location + FireOffset` lands at the
   hand, then restore. Invasive and interacts with collision/physics; likely rejected.

**Decision to take before building (owner: user):** which of 1/2/3, and whether it differs for
hitscan vs projectile weapons. Recommendation to start: **Option 1 (accept)** for the first
playable pass, revisit for projectiles only if it reads badly in-headset. This keeps phase 5
scoped to the direction split.

---

## Risks / same-tick side effects of writing `ViewRotation` (audit before coding)

Each of these is a way the per-frame `ViewRotation = hand` write in Option 1 could leak past aim.
None is verified yet — this is the checklist for the implementation session:

- **Body facing / mesh yaw.** `PlayerPawn.UpdateRotation()` copies `ViewRotation` onto the body
  `Rotation`. If the pawn's visible body/animation or its collision heading turns to face the
  hand, that is wrong. Mitigations: restore body `Rotation` alongside `CameraRotation`, or only
  write `ViewRotation.Pitch`/`Yaw` used by aim and put the body back. **Check** what reads body
  `Rotation` in VR (third-person mesh isn't drawn, but movement and some script might).
- **Movement heading.** VR movement writes `aBaseY`/`aStrafe` directly from a stick heading
  measured against the *controller or head* (`UpdateMovement`, `RRenderScene`), not
  `ViewRotation` — so it *should* be immune. **Confirm** nothing in the movement path re-derives
  forward from `ViewRotation`/body `Rotation`.
- **Pitch clamp.** Script usually clamps `ViewRotation.Pitch` to a limited up/down range. A hand
  aimed straight up/down may be clamped by `UpdateRotation`, capping how steeply you can aim.
  **Check** the clamp and whether it fights hand aim; may need to write aim *after* the clamp, or
  accept the cap.
- **`DesiredRotation` / turning inertia.** Writing `ViewRotation` every frame may interact with
  the script's own smoothing (`aTurn`, `DesiredRotation`). Snap turn already writes
  `ViewRotation.Yaw` successfully, so yaw is known-safe; pitch is the new axis.
- **View restore ordering vs the VR menu.** `CameraRotation` is already frozen/restored for the
  menu plane (`VRMenuFrozenCameraRotation`). Write B must not fight that — only override
  `CameraRotation` on gameplay frames, not while `bNoDrawWorld()`.
- **Roll.** The hand can roll freely; `ViewRotation.Roll` into aim is meaningless and may tilt
  something. Zero the roll on the aim write.
- **Non-`PlayerPawn` viewers** (spectator, cameras, cinematics). Gate the whole thing on a live
  `UPlayerPawn` with a weapon and VR active, like phase 4 does.

---

## Headset-less verification (what can be proven before the Index is back)

Per the research notes, no OpenXR runtime is reachable without the hardware, so the hand pose
can't be exercised unattended. But the *plumbing* can:

- **Write B in isolation:** force `CameraRotation` override with a synthetic anchor yaw and a
  fake "hand" rotation on desktop (VR inactive path), confirm the rendered view follows the
  anchor and not the fake hand. Same forcing trick used to verify `ApplyKeybindings`.
- **Write A side effects:** temporarily set `pawn->ViewRotation` to a fixed non-body direction on
  desktop and dump, per tick, `pawn.Rotation`, `pawn.ViewRotation`, `aBaseY/aStrafe`, and whether
  a test fire's trace direction changed — this surfaces the body-facing / movement-heading /
  pitch-clamp risks above without a headset. Drive via a temporary hook, dump to **stderr**
  (timeout'd runs write no log — see research notes), read `FlashCount`/trace results.
- **Origin mismatch:** measurable on desktop by reading the spawned projectile/trace start vs the
  pawn location; confirms option-1 parallax magnitude numerically before deciding.
- **What still needs the headset:** whether hand-aim *feels* right, the pitch-clamp ceiling in
  practice, and the projectile-from-chest readability that drives the origin decision.

---

## Open questions to settle before writing code

1. **Origin policy** (accept / move `FireOffset` / nudge location), and whether it splits by
   weapon type. *Recommend: accept, for the first pass.*
2. Is `FireOffset` writable from C++ (in `PropertyOffsets`, per-weapon, not overwritten each
   fire)? Determines whether option-2 origin is even available.
3. Does `PlayerPawn.UpdateRotation()` clamp pitch in the shipping UT99/Unreal Gold scripts, and
   at what limit? Determines the achievable aim cone and where Write A must sit.
4. Does anything in the VR movement path re-derive heading from `ViewRotation`/body `Rotation`
   (expected no — it uses controller/head), which would make the body-facing side effect also a
   movement bug?
5. Should the anchor yaw that snap turn advances become an explicit engine-owned field, or keep
   riding `CameraRotation` with Write B restoring it? (Cleaner as an explicit field, given aim now
   scribbles on the shared variable.)

---

## Suggested build order (once the questions above are answered)

1. Introduce the explicit anchor yaw (or confirm `CameraRotation` restore is enough); wire
   snap/smooth turn to it. Verify view unchanged on desktop.
2. Write B: override `CameraRotation` from the anchor after `PlayerCalcView`, gameplay frames
   only. Verify view still ignores a synthetic hand rotation.
3. Write A: set `pawn->ViewRotation` from the weapon-hand aim each gameplay tick. Audit and
   neutralise each same-tick side effect from the Risks list on desktop.
4. Hardware pass: confirm hand aim feels right, tune pitch handling, decide the origin policy for
   projectiles in-headset.
5. Log the framing correction back into `VR Build research notes.md` and tick phase 5 in the
   checklist.
