# WP-1 handover — 2026-07-19 session

Session goal: execute [`WP1-SaveLoad-Plan.md`](WP1-SaveLoad-Plan.md) (save/load persistence) and verify it,
first via CLI/automated testing, then in-headset. Below is what shipped, what's confirmed, and what's left.

## Shipped and confirmed working (in-headset, by the user)

Phase 1 (state-frame serialization) and the deterministic parts of Phase 2 (`Engine::LoadFromSaveFile`) are
implemented. Five real bugs were found during verification — the first automated pass (a save taken right
after a fresh spawn) missed all of them, because a save made *mid-action* (actor mid-latent-call, mid-motion)
is what actually exercises this code. All five are now user-confirmed fixed in-headset:

1. **Null `StateFrame` crash** — `VisibleMesh::DrawDebugInfo` dereferenced `pawn->StateFrame->LatentState`
   with no null check. Harmless before this work (every actor always had *some* `StateFrame`); after phase 1,
   a pawn with no active state at save time correctly gets `StateFrame == nullptr`, and the first such pawn
   drawn crashed the renderer. Fixed with a null check.
2. **Double-pawn problem** — loading a save still called `GameInfo.Login`, which always spawns a fresh pawn
   at a PlayerStart (the reconstructed `LevelInfo->URL` after a load has no `Name=`/`Class=` options to give
   it, so the second login used an empty name). The player ended up controlling the fresh spawn while the
   real, correctly-positioned saved pawn sat orphaned — this was the direct cause of "fell through the mover"
   and "can't retrigger movers" symptoms. Fixed with `Engine::PossessSavedPlayer()`, which finds the already-
   deserialized `bIsPlayer()` pawn and possesses it directly, skipping `GameInfo.Login`/`PreLogin`/`PostLogin`
   entirely (a load is a state restore, not a new connection).
3. **`ObjectFlags::HasStack` never set dynamically** — this flag gates whether `UObject::Save`/`Load` write/
   read an actor's script state at all, and it was purely inherited from whatever the *original `.unr` file's*
   export table said. Most actors start a level idle (no baked-in stack) and only acquire a real `StateFrame`
   when triggered during play (a door opening) — since `Flags` never gained `HasStack` at that point, the next
   save silently wrote nothing for their state, regardless of what they were actually doing. This is why a
   mover triggered and then saved mid-`Sleep()` came back with `StateFrame == nullptr` and permanently stuck
   (`Trigger` disabled forever, since the script never resumed to the point that re-enables it). Fixed:
   `UObject::GotoState` now sets `Flags |= ObjectFlags::HasStack` whenever an actor enters a real state.
4. (Investigated and ruled out as a **false alarm**, not a bug: a mover with `bTriggerOnceOnly=true`
   correctly calls `GotoState('')` after opening once and goes permanently dormant, per `Mover.uc`'s own
   documented design. Looks identical to "frozen" from the outside but is correct UE1 behavior.)
5. **`BasedActors` never serialized** — the list a mover uses to know who's riding it (so it can carry them
   along via `TryMove`) is a plain native C++ member, never serialized, unlike the reverse pointer
   `ActorBase()` (what am I standing on), which *is* a normal property. After a load, a rider's own
   `ActorBase()` correctly points at the mover, but the mover has no idea anyone's on it — so it moves out
   from under them. This was the elevator fall-through. Fixed with `UActor::RelinkBasedActor()`, called from
   `Engine::LinkActorsToLevel` (shared by both `LoadMap` and `LoadFromSaveFile`) to reconstruct the reciprocal
   relationship for every actor whose `ActorBase()` is already set after a load. **User-confirmed fixed with
   the exact original repro** (standing on the elevator while it was actively moving, at the moment of
   saving).

Diff touches: `Engine.cpp/h`, `UObject/UObject.cpp/h`, `UObject/UActor.cpp/h`, `Render/VisibleMesh.cpp`,
`VM/Bytecode.h`, `VM/NativeFunc.cpp/h`, `Collision/TopLevel/CollisionSystem.cpp` (transiently, cleaned up).
All temporary diagnostic/env-var test hooks were added and removed per iteration — none remain in the tree.

## New bug found this session — RESOLVED 2026-07-20

**BUG-007**: saving intermittently crashes the engine. **Fixed and user-confirmed 2026-07-20** (flatscreen,
multiple movers, consecutive save+load in different map areas).

Root cause: `UObject::Save` dereferenced `StateFrame->Func` with no null check. A `StateFrame` can outlive
its state — `UObject::GotoState("")` keeps the frame and calls `SetState(nullptr)`, and `Frame::SetState`
does not touch `LatentState`, which defaults to `Continue` and is never `Stop`. So a dormant actor satisfied
`StateFrame && LatentState != Stop` and crashed on `Func->Code`. This was fallout from fix #3 above: before
`GotoState` began setting `HasStack`, dormant actors mostly lacked the flag and skipped the block entirely.
The intermittency was just whether any actor happened to be dormant-with-a-frame at save time — the common
producer being a `bTriggerOnceOnly` mover, i.e. the same behaviour ruled a false alarm in #4. That ruling
was right (it *is* correct UE1 behaviour); it simply also armed this crash.

The suspicion that it was specific to loaded sessions was the right scenario for the wrong reason: a loaded
session is just one in which more actors have already gone dormant. The load path itself was not involved.

Fixed by guarding on `StateFrame->Func` and on `Func->Code` (a `shared_ptr`, null for a state with no
bytecode). Both write a null `func`, so `Load` restores the actor with `StateFrame == nullptr` — dormant,
which is the case fix #1 made legal. Also replaced `IndexForLatentAction[...]` with a `find()`: `operator[]`
silently inserted a `{state, 0}` entry into the static map for unregistered states (`Continue`, `Stop`).

## Not yet done (per the original plan's own gating)

- **Phase 3** (TravelType gate for BUG-004, Translator-loss-on-travel investigation, BUG-003
  inventory-after-load-travel) — the plan itself requires watching a real level transition and reading the
  log before writing any code; not guessable from source alone.
- **Phase 4** (BUG-005, save package completeness) — explicitly lowest priority, scoped only after 1-3 hold.
- **BUG-002** (glass breaking) — needs actual player input to verify, not automatable and not yet confirmed
  in-headset (movers were the focus this session).
- **UT99 v436 regression pass** — everything above was verified against Unreal Gold only. The plan calls for
  the same save/load/travel loop in UT99 since it shares every code path touched here.

## Also noted, unrelated to WP-1

A pre-existing crash (`VulkanRenderDevice::~VulkanRenderDevice` segfaulting in the AMD RADV driver on every
clean engine shutdown) was found incidentally via `coredumpctl` and confirmed to predate this session by
hours — not caused by this work, not yet filed as a bug, not investigated. Worth a `Bugtracker.md` entry if
picked up (would fit WP-5 or WP-6).

## Where to look for more detail

- Full session narrative and technical reasoning: auto-memory `wp1_saveload_status.md` (and
  `feedback_save_test_scenarios.md` for the testing-methodology lesson learned).
- Original phase-by-phase plan: [`WP1-SaveLoad-Plan.md`](WP1-SaveLoad-Plan.md) — still the reference for
  Phase 3/4 scope and reasoning.
- Debug workflow notes (headless `SE_AUTOLAUNCH` testing, `coredumpctl` usage, decompiling game UnrealScript
  via SurrealDebugger): auto-memory `debug_runtime_workflow.md`.
