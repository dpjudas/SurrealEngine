# WP-3 — Movers, collision and physics: execution plan

**Created 2026-07-20.** Working document, to be retired into `Bugtracker.md` when the package closes
(same convention as the WP-1 docs).

## Why this package

Execution order in `Bugtracker.md` is WP-1 → WP-2 → WP-3. Against the constraint *"no VR hardware, must be
verifiable on flatscreen"*:

| Candidate | Verdict |
| --- | --- |
| **WP-1 remainder (BUG-005)** | Not VR, but it is a single leftover item of a closed package (package writer completeness), not a package. Worth doing, but it is bounded, low-yield work — the games already load and save. Keep as filler. |
| **WP-2 weapon fire semantics** | 2 of its 3 items (BUG-011 aim indicator, BUG-012 `FireOffset` at the hand) are VR-only by definition and cannot be verified without a headset. Only BUG-010 is flatscreen-testable. **Blocked as a package.** |
| **WP-3 movers, collision and physics** | All 7 items are pure engine physics, reproducible and verifiable with keyboard and mouse on Unreal Gold and UT99. **This is the package.** |

So: **execute WP-3.** BUG-010 (charging weapons) is a natural flatscreen pick-up if WP-3 finishes early —
it is listed as an appendix below, not as part of the package.

## What the investigation already found

I read the collision and physics code before writing this plan. Five of the seven items already have a
concrete, named root-cause hypothesis. That changes the shape of the work: this is not an open-ended
"physics feels bad" package, it is mostly four specific defects plus one genuine feature gap.

### Finding A — world AABB collision ignores semisolid geometry entirely

`TraceAABBModel::Trace` (`SurrealEngine/Collision/BottomLevel/TraceAABBModel.cpp:16`) and
`OverlapAABBModel::TestOverlap` (`.../OverlapAABBModel.cpp:15`) only ever collide against **leaf hulls**
(`node->CollisionBound >= 0`). Leaf hulls exist for CSG-solid leaves. Semisolid brushes (`PF_Semisolid`,
`ULevel.h:132`, part of `PF_AddLast`) deliberately do **not** carve solid space — they contribute BSP nodes
and surfaces but no solid leaf, so an extent trace can never hit them.

Corroborating evidence:

- `grep PF_Semisolid` over `Collision/` returns **nothing**. The collision layer has no concept of it.
- `TraceAABBModel::NodeAABBIntersect` / `TriangleAABBIntersect` (`TraceAABBModel.cpp:188` and `:116`) are
  fully implemented, check `PF_NotSolid`, and are **called from nowhere** (`grep` confirms: declared in the
  header, defined, never invoked). That is exactly the missing per-node polygon test.
- The ray paths (`TraceRayModel`) *do* walk node polygons — which explains the asymmetry players report:
  shots and traces hit semisolid railings and catwalks, the player's cylinder falls through them.

This predicts BUG-021 exactly, and is a strong candidate for BUG-026 (catwalks in the DM-Conveyor lava room
are the classic semisolid use case).

> **REFUTED 2026-07-20 by measurement — do not implement the fix this finding proposes.**
>
> Phase 2 opened with the diagnostic step the plan required, and the diagnostic killed the hypothesis.
> A level-wide probe (`SE_DEBUG_SEMISOLID=1`) walks every semisolid polygon in the loaded map, fires a ray
> and an extent sweep down the polygon's own normal from 96 units out, and compares where each stopped —
> using the ray as the control, since it walks node polygons and demonstrably stops on the polygon itself.
>
> | Map | Semisolid nodes | Probed cleanly | Sweep passed through | Sweep stopped on the surface |
> | --- | --- | --- | --- | --- |
> | Vortex Rikers (Vortex2) | 2701 of 8559 | 1546 | **6** | 1222 |
> | NyLeve's Falls | 2848 of 5990 | 2190 | **9** | 1898 |
>
> (Small 4-unit probe box; the remainder stop early on neighbouring geometry, which is inconclusive by
> construction in tight interiors. The player-sized box gives the same verdict with more inconclusive
> probes: 0 through in Vortex2, 4 in NyLeve.)
>
> Extent sweeps **do** collide with semisolid geometry — leaf hulls cover it in practice, so the missing
> `PF_Semisolid` handling in the collision layer is not the defect, and wiring `NodeAABBIntersect` in would
> have been at best a no-op and at worst a performance regression and a source of duplicate hits.
>
> What survives is much narrower and more useful: a *small minority* of semisolid polygons are genuinely
> not swept. In NyLeve all four player-box failures cluster at one spot, around **(-4127, -5410, -6000)**,
> where a live runtime sweep was also caught crossing two polygon-intersected semisolid nodes and returning
> no hit. Vortex2's failures are all steeply slanted, non-axis-aligned faces
> (e.g. normal `(0.73, -0.15, 0.67)` at `(-2253, 954, -219)`).
>
> **DEPRIORITIZED 2026-07-20 at the user's direction.** BUG-021 was reported by someone else and the user
> has never reproduced it themselves. With no first-hand repro available and the general mechanism measured
> as working, the phase is parked rather than blocked — nobody involved can currently exercise it. The
> `SE_DEBUG_SEMISOLID` diagnostic and the failing coordinates above are kept so whoever does hit it can
> pick this up cold.
>
> ~~**Revised Phase 2:** BUG-021 is real but localized, so it needs a real in-game repro before any fix —
> the general mechanism works and a blind fix would be aimed at nothing. Next steps, in order:
> 1. Get the user's actual fall-through location (they reported the symptom; the synthetic probe cannot
>    tell which of these polygons a player stands on).
> 2. Characterize the failing minority. Both leads point at hull geometry rather than flags: steep slanted
>    faces suggest the bevel-plane approximation in `SweepCursor::ClipBevel` losing a hull, and the NyLeve
>    cluster suggests a specific brush whose leaf hull is missing or degenerate.
> 3. Only then decide whether the fix is per-node polygon testing (as originally proposed, but applied
>    narrowly), a bevel-plane correction, or a hull-generation issue.~~
>
> The `boxEpsilon` shave at `TraceAABBModel.cpp:29-32` (0.1 units off every hull, added so ammo pickups
> stop falling through floors) is a third candidate worth checking against the failing polygons.

### Finding B — `EncroachingActors` tests exactly one actor per call

`OverlapTester::EncroachingActors` (`Collision/TopLevel/OverlapTest.cpp:235-237`) reads and writes
`actor->Collision.CheckCounter` — the **mover's own** counter — where every sibling function uses
`testActor->Collision.CheckCounter`. Consequence: the first candidate in the first bucket sets the mover's
counter, and every subsequent candidate fails the `if` and is skipped. A mover therefore notices at most one
encroaching actor per move, chosen arbitrarily by hash-bucket order.

This is the sole feed for `EncroachingOn` / `EncroachedBy` in `UActor::TryMove` (`UObject/UActor.cpp:1659-1705`),
i.e. for movers pushing, crushing and refusing to close on the player. Prime suspect for BUG-022.

### Finding C — `isMoving` uses `&&` where it must be `||`

Three copies, all identical:

```
bool isMoving = (vel.x != 0.0f && vel.y != 0.0f);
```

`UActor.cpp:546` (walking), `:834` (swimming), `:951` (flying). If either horizontal component is exactly
zero, the whole movement block is skipped.

- Walking: mostly masked, since float velocity is rarely exactly axis-aligned — but reachable via
  `ZoneVelocity`, conveyors and scripted movement.
- **Swimming and flying: not masked at all.** Vertical-only motion has `vel.x == vel.y == 0`, so swimming
  straight up or down does nothing; the player only rises when they also drift sideways. This is a direct
  candidate for BUG-025 ("underwater collisions are buggy"), which may turn out to be a movement bug rather
  than a collision bug.

Correct form: `vel != vec3(0.0f)` for swimming/flying, `(vel.x != 0.0f || vel.y != 0.0f)` for walking.

### Finding D — walking physics has no answer for a blocking actor

`UActor::TickWalking` (`UActor.cpp:571-609`): when the move is blocked, the wall-slide branch is guarded by
`else if (hit.Normal.z < 0.2f ...)`, reached only when `hit.Actor` is null (world) or the walker is not a
player. When a *player* hits an actor that is neither a pushable decoration nor movable, the code is:

```
else if (hit.Actor->bCollideActors() && ...)
{
    // TODO: We hit a non-movable actor
}
```

— an empty branch. No slide, no velocity projection. The pawn dead-stops against a crate or another pawn and
keeps re-colliding every tick. Then, three statements later, the step-down probe runs; whether the player is
declared falling depends on that probe succeeding while jammed. That is the mechanism behind the second half
of BUG-020 ("on breaking free dies from phantom fall damage"): fall damage is script-side off `Velocity.Z`
at `Landed`, and `Velocity` is recomputed as `(Location - OldLocation) / elapsed` at `:645` — a jam followed
by a sudden release produces a large synthetic velocity that the script reads as a fall.

Secondary suspicion to confirm during the work: `TraceTester::RayCylinderTrace` starting *inside* the other
cylinder. If it returns `t == 0` while overlapped, the pawn can never move out in any direction — a hard
stuck rather than a soft one. There is a live comment at `TraceTest.cpp:256` questioning exactly this family
of code ("Shock rifle combo blasts does not work with this version. Why?"). No depenetration/push-out step
exists anywhere.

### Finding E — movers teleport, they do not sweep

`UActor::TryMove` skips the entire trace when the actor has a brush (`UActor.cpp:1606`, `if (!Brush())`), so
a mover never sweeps its path; it jumps to the destination and *then* runs an overlap test at the new
position. `TickMovingBrush` (`:1289`) applies `SetRotation` only when the translation reported
`Fraction == 1.0f`, and rotation itself is applied with no collision consideration at all.

Consequences to verify:

- A fast mover crossing an actor within one tick never notices it (contributes to BUG-023 for projectiles
  and to BUG-022 for the player).
- Approach-angle dependence in BUG-022 fits a position-only overlap test combined with Finding B.
- For projectiles specifically also check `TraceTester::TraceActor` (`TraceTest.cpp:162-166`): a mover is
  skipped outright unless `traceWorld` is set, and the mover branch converts the trace into brush-local
  space with a hand-rolled scale/rotation transform (`:174-198`) whose `vec4(dir, 1.0f)` direction transform
  is only safe because the matrix is pure rotation. Fragile; worth an explicit review, since BUG-023 says
  *some* movers, i.e. probably the rotated or non-uniformly scaled ones.

### Finding F — no hypothesis yet for BUG-024

Mover buttons being too easy to push (the Vortex Rikers Kevlar-room ceiling button firing when the player
walks *under* it) has not been traced yet. Two leads worth checking first: the mover's collision insertion
uses the **whole brush bounding box** as its bucket extent (`CollisionSystem.cpp:116-125`), and the touch
path is cylinder-vs-cylinder (`OverlapTest.cpp:286-291`) with no brush-shape awareness. Either would make a
flat ceiling plate behave as a tall box.

## Execution phases

Ordering principle: **shared foundations before symptoms.** Findings A–C are single-site defects that several
of the reported bugs sit on top of; fixing them first may collapse the list, and doing them last would mean
debugging symptoms through a broken substrate.

Every phase ends in its own commit, so any phase can be bisected out on its own. `Bugtracker.md` is updated
in the same commit that fixes an entry (WP-1 convention).

### Phase 0 — repro baseline (no code changes)

Establish that each bug reproduces *before* touching anything, and write down the exact route for each. This
is the regression suite for the whole package, since the repo has no automated tests.

Routes to pin down:

| Bug | Proposed route |
| --- | --- |
| BUG-020 | Vortex Rikers: walk into a crate/barrel and into a stationary pawn; push along a wall. Watch for dead-stop and for health loss after release. |
| BUG-021 | Any map with semisolid railings/walkways. Identify a concrete one during this phase and record the map + coordinates. |
| BUG-022 | Vortex Rikers doors; approach on the diagonal and at the hinge side while closing. |
| BUG-023 | Fire slow (Dispersion) and fast (ASMD/rocket) projectiles at closed doors and at moving lifts. |
| BUG-024 | Vortex Rikers Kevlar Suit room: walk under the ceiling button without looking up. |
| BUG-025 | Unreal Gold water: swim straight up and straight down with no horizontal input. Confirms/denies Finding C before any fix. |
| BUG-026 | UT99 DM-Conveyor lava room catwalks. |

Deliverable: `WP3-Repro-Routes` section appended to this file, with map names and what "wrong" looks like.
Also confirm the build baseline: `SurrealEngine` builds clean, and remember the Vulkan teardown SIGSEGV on
every quit is BUG-044 and pre-existing — it is **not** a regression from this work and must not poison a
bisect.

### Phase 1 — the three single-site defects (Findings B and C)

Small, independent, high confidence.

1. `OverlapTest.cpp:235-237` — use `testActor->Collision.CheckCounter`. Also move the
   `testActor == actor || testActor->Brush()` guard *before* the counter write, matching the sibling
   functions, so a skipped actor is not marked as visited.
2. `UActor.cpp:546/:834/:951` — fix `isMoving`. Walking keeps the horizontal-only form (`Velocity().z` is
   forced to zero there anyway); swimming and flying must consider `z`.
3. Re-run the Phase 0 routes for BUG-022 and BUG-025 and record what changed.

Expected: BUG-025 substantially or entirely fixed; BUG-022 improved but probably not closed (Finding E
remains). Both may reveal follow-on behaviour that was previously hidden — e.g. movers that now correctly
report encroachment may start crushing the player where they previously ignored them. That is progress, but
it needs a UT99 + Unreal Gold pass before the commit.

### Phase 2 — semisolid world collision (Finding A) → BUG-021, likely BUG-026

**Status: hypothesis refuted, phase re-scoped — see the block under Finding A.** Step 1 below was
executed and returned a negative; steps 2-5 are retained only for the record and must not be executed as
written. The phase is now blocked on a real in-game repro.

The substantive collision work.

1. Confirm the diagnosis cheaply first: log, for a known fall-through spot, whether any node with
   `PF_Semisolid` is traversed while zero leaf hulls are hit. Do not start the fix until the trace
   confirms it. **DONE — it does not confirm it.**
2. Wire the existing `NodeAABBIntersect` into `TraceAABBModel::Trace`: for each traversed node whose surface
   carries `PF_Semisolid` (and not `PF_NotSolid`), run the polygon test and push a hit. The function already
   exists and already handles the `PF_NotSolid` exclusion — the working assumption is that it was written
   for precisely this and never hooked up.
3. The hit **normal** matters as much as the hit: walking, step-up and the `hit.Normal.z >= 0.7071f` floor
   test all read it. `NodeAABBIntersect` returns only a fraction, so the node plane normal must be taken from
   the node (flipped to face the trace origin) and filled into the `CollisionHit`.
4. Mirror the same node test in `OverlapAABBModel::TestOverlap`, or `CheckLocation`/spawn placement will
   still consider semisolid space empty.
5. Performance check: this adds a per-node polygon test to every extent trace, i.e. to every pawn move.
   Gate it on `PF_Semisolid` so ordinary solid geometry costs one flag test, and eyeball the frame time in a
   large map (NyLeve's Falls) before and after.

Risk: this is the one phase that changes collision for *every* map surface. If frame time or stability
regresses, it is the first thing to revert. Keep it as a standalone commit.

### Phase 3 — actor-blocking in walking physics (Finding D) → BUG-020

1. Fill in the empty branch at `UActor.cpp:584`: on a blocking actor hit, project the remaining move along
   the hit normal exactly as the world-hit branch does, and raise `HitWall`/`Bump` consistently. UE1's own
   behaviour is to slide along other pawns and non-pushable decorations, not to stop dead.
2. Verify the overlapping-start case in `RayCylinderTrace`: construct the situation (two pawns spawned
   overlapping, or a decoration moved onto the player) and check the returned `t`. If it is `0`, add a
   push-out: when the sweep starts already penetrating, move along the separation axis rather than
   reporting a zero-fraction block.
3. Re-check the phantom fall damage after 1 and 2. If it persists, instrument `Velocity` at `:645` and at
   the `Landed` call, and establish whether the spike comes from the jam-release or from the step-down
   probe misfiring while jammed. Do not guess — this is the one symptom in the package with two plausible
   mechanisms.

### Phase 4 — movers (Finding E) → BUG-022 remainder, BUG-023

1. Give brush actors a real swept move. The cheapest correct-enough form: substep `TickMovingBrush` so a
   mover never advances more than a fraction of the smallest colliding actor's radius per step, then run
   the existing position-based encroachment test at each substep. A full brush-vs-cylinder sweep is the
   textbook answer but is a much larger change — take it only if substepping demonstrably fails.
2. Apply rotation through the same path as translation (`UActor.cpp:1289`): a rotating mover currently
   changes orientation with no encroachment consideration whatsoever, and `TurnBasedActors` (`:1422`) is
   the only thing that reacts.
3. BUG-023 specifically: check `TraceActor`'s mover branch (`TraceTest.cpp:162`) against non-uniformly
   scaled and rotated movers, and confirm projectiles reach it at all (`traceWorld` must be set for a mover
   to be considered — verify that projectiles trace with it). Also check whether fast projectiles tunnel:
   `TickProjectile` (`UActor.cpp:1052`) does exactly one `TryMove` per tick with no substepping, so a
   rocket at 1000+ uu/s versus a thin door is a real tunneling candidate independent of the mover code.

### Phase 5 — trigger sensitivity (Finding F) → BUG-024

Only start once Phases 1–4 have settled; a mover that now sweeps and reports encroachment correctly may
change this symptom on its own. Then: trace what actually fires the Vortex Rikers ceiling button
(`Bump` from `TryMove`, a `Trigger` actor's proximity check, or a `Touch`), and compare the shape used
against the brush. The bounding-box bucket insertion at `CollisionSystem.cpp:116-125` and the
cylinder-only touch test are the two leads.

### Phase 6 — regression pass and close-out

WP-1's most valuable lesson: **run the pass on both games.** BUG-008 was a hard crash on every UT99 save
that Unreal Gold structurally could not expose, found only because a second game was tested.

1. Full route replay from Phase 0 on **Unreal Gold v226** and **UT99 v436**.
2. Cross-package regression, because collision touches everything: save → load → travel still works (WP-1);
   pickups still land on floors rather than through them (the `boxEpsilon` at `TraceAABBModel.cpp:29-32`
   exists specifically to stop ammo falling through the floor, and Phase 2 changes that code path);
   elevators still carry the player; projectiles still hit walls and pawns.
3. Update `Bugtracker.md`: strike through what is fixed with the root cause and the phase, exactly as WP-1
   entries read. Anything not fixed stays open with what was learned recorded on it.
4. Delete this file in the closing commit and fold anything still live into the WP-3 section.

## Verification notes

- No unit tests exist; every claim above is verified by running the game against a real UE1 install.
- Build and deploy: build in `build/`, then copy `build/SurrealEngine` into `build/Unreal Gold/System/`
  before testing.
- These are all flatscreen scenarios and need no headset, but they do need a human at the keyboard —
  budget real interaction time per run rather than short timeouts.
- VR must keep working even untested: nothing in this plan touches `VR/`, and `VRSubsystem` stays inert
  when disabled. The one place to be careful is Phase 3 — `VRHands.cpp:403` reads `bBlockPlayers` on the
  same actors the walking code does, so keep the blocking semantics rather than redefining them.

## Sequencing summary

| Phase | Targets | Confidence | Size | Status (2026-07-20) |
| --- | --- | --- | --- | --- |
| 0 | all — repro baseline | — | small | folded into the phases below |
| 1 | BUG-025, BUG-022 (partial) | high | small | **BUG-025 user-confirmed fixed** (swims up/down); movers now notice the player |
| 2 | BUG-021, BUG-026 | ~~high~~ **refuted** | medium | **parked** — no first-hand repro exists (see Finding A) |
| 3 | BUG-020 | medium | medium | **user-confirmed fixed** — chest pushes along a wall, no phantom damage |
| 4 | BUG-022, BUG-023 | medium | large | **implemented, awaiting playtest** — see "Phase 4 as built" |
| 5 | BUG-024 | low | unknown | not started |
| 6 | close-out, both games | — | medium | not started |

### Playtest 2026-07-20 (Unreal Gold, first two levels)

Confirmed fixed: swimming straight up and down (BUG-025); pushing a chest along a wall moves the chest and
does no damage to the player (BUG-020); movers notice the player rather than ignoring them (BUG-022, the
encroachment half).

Three regressions found, all introduced by this package, all since fixed in one commit — pickups and
proximity triggers firing only intermittently (the phase 3 depenetration dropped the hit that `Touch` is
sent from), and being thrown off a mover when walking into its solid part (the new actor-slide branch was
missing the wall-slide branch's hit-normal guard).

### The lever / second mover "bounce" is correct UE1 behaviour, not a defect

Retest narrowed it: the second mover *bounces back*, and afterwards needs triggering twice. Reading the
game's own decompiled `Engine.Mover` settles it — `Mover.EncroachingOn` ends in a switch on
`MoverEncroachType`, and the `ME_ReturnWhenEncroach` branch calls `Leader.MakeGroupReturn()` and returns
**true**, i.e. "abort the move". A mover that meets a blocking pawn returns to its previous position and
has to be triggered again. That is the shipped behaviour of the game's script, not something the engine
decides.

So this is the phase 1 encroachment fix *working*, and visible for the first time: with the counter bug,
a mover examined one arbitrary candidate per move and almost always missed the player, so it never
returned. The apparent regression is the feature.

Two things follow, both for phase 4 rather than now:

- **`EncroachingOn` is only reached for actors that block** (`TryMove` filters on
  `bBlockActors`/`bBlockPlayers` first), so triggers and navigation points cannot cause a spurious bounce.
  Decorations are handled by the script's own earlier branch, which destroys them and returns false.
- **`EncroachingActors` scans the wrong bucket range.** It derives `origin`/`extents` from the mover's
  *collision cylinder* (`OverlapTest.cpp:205-208`) while `CollisionSystem::AddToCollision` inserts movers
  by their *brush bounding box* (`CollisionSystem.cpp:116-125`). The source even carries a "To do: is
  radius and height correct for a mover? Should it use the brush bounding box?" comment at that spot. This
  under-reports: a large door scans a small region around its origin and can miss actors it is about to
  encroach on. Worth fixing in phase 4, but it makes movers *more* sensitive, so it should not land
  immediately after a playtest that just validated the current feel.

Also worth confirming during phase 6: no mover in the first two levels crushes the player. Given the above
that is probably map-accurate — `ME_CrushWhenEncroach` is a per-mover authoring choice — but it is cheap to
confirm against a mover known to use it.

### BUG-023 hypothesis for phase 4 (untested — **confirmed**, see "Phase 4 as built")

`TryMove`'s blocking test for a projectile resolves to
`isBlocking = hit.Actor->bBlockPlayers() && bBlockPlayers()` (`UActor.cpp:1616-1619`, projectiles take the
`useBlockPlayers` path). If `Projectile`'s class defaults have `bBlockPlayers` false — likely, since
projectiles are not meant to block anything — then **no** mover can ever block a projectile, because a
mover is an actor and only the world-hit branch is unconditional. That would explain "projectiles pass
through some movers" without any tunneling or transform bug, and it predicts the failure depends on the
projectile class rather than on the mover. Check the class defaults first; the decompiler does not emit
`defaultproperties`, so read them from the loaded CDO rather than from the `.uc`.

Phase 4 is deliberately gated: it is the largest and riskiest change in the package (movers stop teleporting
and start sweeping), and stacking it on two unverified physics changes would make any regression much harder
to attribute. Verify 1 and 3 in gameplay first.

### Phase 4 as built (2026-07-20)

**The BUG-023 hypothesis was correct.** Read from the loaded CDOs via a temporary `SE_DEBUG_BLOCKFLAGS`
probe (since removed):

```
Engine.Projectile   bBlockActors=0 bBlockPlayers=0 bCollideActors=1 bCollideWorld=1
Engine.Mover        bBlockActors=1 bBlockPlayers=1 bCollideActors=1 bCollideWorld=0
Engine.PlayerPawn   bBlockActors=1 bBlockPlayers=1 bCollideActors=1 bCollideWorld=1
```

So `hit.Actor->bBlockPlayers() && bBlockPlayers()` was mover-yes AND projectile-no, and **no mover could
ever block any projectile**. Players were unaffected only because a `PlayerPawn` happens to set both flags,
which is why the symmetric test survived this long.

Four changes:

1. **Brush hits decide blocking from the brush's flags, not both sides** (`UActor.cpp`, `TryMove`). A mover
   is level geometry: whether it stops you is its decision, and yours only insofar as you collide with the
   world at all (`bCollideWorld`). Non-brush actor hits keep the old symmetric rule, deliberately — making
   it one-sided there would let a rocket be blocked dead by the pawn that fired it.
2. **A mover hit now reaches `HitWall`** (`TickProjectile`). The guard was `!hit.Actor`, so once change 1
   let a door stop a rocket, the rocket would have halted in mid-air and never detonated. `HitWall` already
   takes the actor it struck for exactly this case.
3. **`EncroachingActors` searches by the brush bounding box** (`OverlapTest.cpp`), matching how
   `AddToCollision` files movers, and answering the "To do" that was sitting at that spot. Previously a
   wide door searched a small region around its origin and never noticed anyone near its ends.
4. **Movers advance in bounded steps and turn through the encroachment path** (`TickMovingBrush`). The
   per-tick advance is now capped at 16 units of travel (`maxAlphaStep`, floored at 1/64 of a leg so the
   loop always terminates), because `TryMove` still does not sweep a brush — it moves the whole delta and
   then asks who ended up inside — so a fast mover could step clean over an actor, overlapping it at
   neither sample. Rotation is now applied *before* `TryMove` and reverted if the move is refused; the
   tiny-delta early-out in `TryMove` is skipped for brushes so that a mover which only turns still gets
   its encroachment test. That early-out is also what refreshes the collision buckets after a turn.

Smoke-tested only: Unreal Gold Vortex2 and NyLeve, and UT99 DM-Conveyor, each run to a 40s+ timeout with no
crash and no exception. **Everything here needs a real playtest** — 4 in particular changes how every mover
in the game moves, and 1 changes what stops a projectile.

What to look for: rockets and other projectiles now explode against doors and lifts instead of passing
through (BUG-023); lifts and doors still carry the player and still bounce/return correctly rather than
stuttering or stalling; nothing gets crushed or shoved that did not before; rotating movers, if any appear
in the first levels, no longer sweep through the player.

## Appendix — BUG-010 (WP-2), if WP-3 finishes early

Charging weapons mishandle the held trigger; the Rocket Launcher fires one rocket immediately and only then
starts charging. Fully reproducible on flatscreen with UT99. It is a fire-state/latent-function timing
problem in the weapon script path rather than a collision one, so it shares nothing with WP-3 — treat it as
a separate change, not a phase of this package.
