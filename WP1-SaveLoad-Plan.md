# WP-1 — Save/load and level travel persistence: execution plan

Covers BUG-001 … BUG-006 from [`Bugtracker.md`](Bugtracker.md). Target configurations are the two playable
ones: **UT99 v436** and **Unreal Gold v226**.

## Why these six are one work package

They are not six independent bugs. Reading the save path top to bottom:

`UObject::Save` (`SurrealEngine/UObject/UObject.cpp:126`) writes a **zeroed** execution stack for every
object carrying `ObjectFlags::HasStack`:

```cpp
if (AllFlags(Flags, ObjectFlags::HasStack))
{
#if 1
    stream->WriteIndex(0);   // func
    stream->WriteIndex(0);   // state
    stream->WriteInt64(0);   // probeMask
    stream->WriteInt32(0);   // latentAction
#else
    ... the real implementation, compiled out ...
#endif
}
```

`UObject::Load` (same file, line 57) reads those four fields back and only builds a `StateFrame` when
`func && state` are both non-null. Zeros mean **no actor in a loaded level has a script state frame at all.**

That single fact explains the cluster:

- **BUG-002** — a mover sits in its `Trigger`/`Interpolating` state in the original save; on load it has no
  state frame, so `Trigger` dispatches nowhere and it never moves. Breakable glass is the same shape —
  its script state is gone, so the damage handler is unreachable.
- **BUG-001** — script code and native code that assume a live `StateFrame` (state-scoped event dispatch,
  latent actions, `GotoState` from a null frame) run a few ticks later and dereference null.
- **BUG-003** — `Engine::CreateTravelInfo` walks `Level->Actors` for the player pawn's travel properties.
  With the loaded level's actors in a degraded state, the inventory chain it walks is unreliable.

BUG-004 (Translator lost on travel) and BUG-006 (stale `GameInfo`) are adjacent and cheap to fix while the
same code is open, so they ride along.

**The whole package is gated on one thing: reinstating real state-frame serialization.** Do that first.

---

## Phase 1 — Make the state frame serializable (blocks everything else)

The `#else` branch in `UObject::Save` is not merely disabled, it **does not compile**. Three symbols it
references do not exist. Add them before touching the `#if`.

### 1.1 `NativeFunctions::IndexForLatentAction`

`SurrealEngine/VM/NativeFunc.{h,cpp}` has only the forward map:

```cpp
static Array<LatentRunState> LatentActionByIndex;   // NativeFunc.h:19
```

built by `RegisterLatentAction(int nativeIndex, LatentRunState)` (`NativeFunc.cpp:52`). Add the reverse map
as `static std::map<LatentRunState, int> IndexForLatentAction;` and populate it in the same
`RegisterLatentAction` body — one line, and it cannot drift from the forward map because both are written
in the same place.

While in `UObject::Load`, fix the **unbounded index** at line 92:

```cpp
StateFrame->LatentState = NativeFunctions::LatentActionByIndex[latentAction];
```

`latentAction` comes straight off disk. Bounds-check it against `LatentActionByIndex.size()` and fall back
to `LatentRunState::Continue` with a `LogMessage` on a bad value — a corrupt or foreign save must not read
out of bounds.

### 1.2 `Bytecode::FindOffset(int statementIndex)`

`SurrealEngine/VM/Bytecode.h` has `FindStatementIndex(uint16_t offset)` built on the private
`std::map<uint16_t, Expression*> OffsetToExpression`, but no inverse. Add:

```cpp
int FindOffset(int statementIndex) const;   // -1 if not found
```

Implement by scanning `OffsetToExpression` for the entry whose `second->StatementIndex == statementIndex`.
A linear scan is fine — this runs once per saved actor, not per tick. Note `FindStatementIndex` itself
dereferences `find()`'s result without checking `end()`; harden it in the same pass, since it is now being
fed offsets from a save file rather than only from trusted package data.

### 1.3 `UObject::IsEventDisabled`

`UObject.h:311` has `IsEventEnabled`, and `DisabledEvents` is a
`std::map<NameString, std::set<NameString>>` **keyed by state name** (see `EnableEvent`/`DisableEvent`,
`UObject.h:313-323`). The `#else` branch calls a non-existent `IsEventDisabled`. Either add that trivial
inverse or rewrite the probeMask loop against `IsEventEnabled`. Whichever — it must read the set for the
state being saved, i.e. `GetStateName()` at save time.

### 1.4 Fix the probeMask read loop

`UObject::Load:69-75` is wrong twice:

```cpp
for (int i = 0; i < 64; i++)
    if (((1ULL >> (uint64_t)i) & 1) == 1)      // >> should be <<, and probeMask is never consulted
        DisableEvent(ToNameString((EventName)i));
```

`1ULL >> i` is 1 only for `i == 0`, so this unconditionally disables event 0 and ignores `probeMask`
entirely. Correct to `if ((probeMask >> i) & 1)`.

Second, ordering: the loop runs **before** `StateFrame` is assigned, so `DisableEvent`'s
`GetStateName()` returns the empty name and every disabled event is filed under `""` instead of under the
loaded state. Move the probeMask loop to **after** the `StateFrame` is built.

### 1.5 Flip the `#if 1` and reconcile func/state

Enable the real branch. One asymmetry to resolve deliberately rather than by accident: the writer derives

```cpp
UStruct* func  = StateFrame ? StateFrame->Func : nullptr;
UState*  state = StateFrame ? UObject::TryCast<UState>(StateFrame->Func) : nullptr;
```

so `state` is null whenever the frame sits in a plain function rather than a state — while the reader
requires `func && state` to rebuild anything. Decide which is authoritative and make the two agree; the
safe reading of UE1 semantics is that `state` is the actor's current state and `func` is where execution
is suspended, and they coincide only for a state-body latent wait. Document the choice in a comment at
`UObject::Save`, because the next person will hit exactly this.

### 1.6 Verify phase 1 in isolation

Before touching anything else: start a map, save, load, and confirm from the log that actors come back with
non-empty `GetStateName()`. `SurrealDebugger` can dump the loaded level's actors. Expect BUG-002 (movers,
glass) to be substantially fixed by this phase alone — that is the checkpoint.

---

## Phase 2 — Complete `LoadFromSaveFile`

`Engine::LoadFromSaveFile` (`SurrealEngine/Engine.cpp:738`) is a stripped-down copy of `LoadMap`. Compare
the two side by side; the load path is missing, with several lines sitting commented out in place:

| Done by `LoadMap` | Present in `LoadFromSaveFile`? |
| --- | --- |
| `LevelInfo->ComputerName`, `HubStackLevel` | commented out |
| `LevelInfo->NetMode`, `DefaultTexture` | commented out |
| `Level->TravelInfo = travelInfo` | **missing** |
| GameInfo spawn + `LevelInfo->Game()` + `engine->GameInfo` | **missing** (BUG-006) |
| `LinkActorsToLevel()` | present |
| `render->OnMapLoaded()` | via `LoginPlayer` |

Work through each:

1. **`engine->GameInfo` (BUG-006).** Assigned only at `Engine.cpp:688` inside `LoadMap`. After a save load
   it still points at the *previous, unloaded* level's GameInfo. The saved package contains its own GameInfo
   actor; re-point `engine->GameInfo` (and confirm `LevelInfo->Game()`) at it after `GetLevelObject()`.
   Decide the correct behaviour when the save has no GameInfo — throw with a clear message rather than
   limping on.
2. **The commented-out `LevelInfo` fields.** Determine for each whether it is *state that must survive the
   save* (leave loaded from the package) or *session state that must be re-established* (must be re-set).
   `DefaultTexture` and `NetMode` are almost certainly the latter. Replace each commented block with either
   real code or a comment saying why it must not be set.
3. **`Level->TravelInfo`.** Needed by phase 3.

### The double-pawn problem

`Engine::Run:270-274` calls `LoadFromSaveFile(url)` and then **`LoginPlayer()`** — the same `LoginPlayer`
the fresh-map path uses, which calls `GameInfo.Login` and *spawns a new player pawn*. But the saved level
already contains the saved pawn, with the player's health, inventory and position.

Establish by experiment which pawn ends up in `viewport->Actor()` and whether the other is left orphaned in
`Level->Actors` — an orphaned pawn that still ticks is a strong second candidate for BUG-001's
"crashes after a few seconds", independent of phase 1. If confirmed, the load path needs its own
possession routine (find the saved pawn, `viewport->Actor() = pawn`, `Possess`, skip `Login`) rather than
reusing `LoginPlayer` wholesale. Keep the VR reset at the end of `LoginPlayer` (`Engine.cpp:939-943`,
`vrHands->Reset()` / `vrWheel->Reset()`) on whichever path is taken — those hold actor pointers from the
level just unloaded.

---

## Phase 3 — Travel transfer (BUG-003, BUG-004)

Only start this once phases 1–2 hold, since a loaded level's actors are the input to travel.

### 3.1 The `TravelType` gate

`Engine::LoginPlayer` (`Engine.cpp:904`) transfers items only when:

```cpp
if (actorActuallySpawned && ClientTravelInfo.TravelType == ETravelType::TRAVEL_Relative)
```

`ClientTravelInfo.TravelType` defaults to `TRAVEL_Absolute` (`Engine.h:169`) and is assigned **only** in
`Engine::ClientTravel`. The `LevelInfo->NextURL()` paths at `Engine.cpp:242-260` — including the
`bNextItems()` branch, which exists precisely to carry inventory — call `LoadMap` + `LoginPlayer` without
ever setting `TravelType`, so they inherit whatever the last `ClientTravel` left behind (or `Absolute` if
there was none) and silently skip the transfer. Same for the save-load path. **Verify this against a real
Unreal Gold level transition before changing it** — the teleporter route goes through
`PlayerPawn.ClientTravel(…, TRAVEL_Relative, true)` and may well be setting it correctly, in which case
BUG-004 is elsewhere and only the `NextURL` paths need the fix.

### 3.2 Instrument the Translator specifically (BUG-004)

`ActorTravelInfo::Accept` (`SurrealEngine/UObject/ObjectTravelInfo.cpp:115`) already logs its two failure
modes — `"could not spawn travel actor with class name: …"` and `"could not find travel actor: …"` — and
`LoginPlayer` logs three distinct skip reasons. **Run the first→second level transition in Unreal Gold and
read the log before writing any code.** The Translator either (a) never entered `CreateTravelInfo`'s
`processList`, (b) failed `FindClass` on its class name, or (c) was spawned but not relinked into the
inventory chain. Each has a different fix; the log distinguishes them.

Candidate root causes to check in that order:

- **The `None` handling** at `ObjectTravelInfo.cpp:96-103`: object properties that are null are deliberately
  *not* written, with a Deus Ex-specific comment explaining why. The chain is walked through the `Inventory`
  property, so an item whose predecessor drops its `Inventory` link vanishes along with everything after it.
  Check what the Translator's position in the chain is and whether anything ahead of it wrote a null.
- **`Accept`'s spawn** at line 132: `pawn->Spawn(cls, pawn, NameString(), {}, {})` passes no location or
  rotation. Confirm an inventory item tolerates that, and that `GameInfo.AcceptInventory` (called at
  `Engine.cpp:929`) relinks it.
- **`GetAllTravelProperties`** (`UObject.cpp:455`) iterates `PropertyData.Class->Properties` matching
  `PropertyFlags::Travel || prop->Name == "Inventory"`. *Already checked:* `UClass::Properties` **is**
  flattened (`UClass.cpp:115` seeds it from `BaseStruct->Properties` before appending), so `Inventory` is
  visible on `Translator` and a missing-inherited-property explanation is ruled out. Recorded here so it is
  not re-investigated.

### 3.3 Inventory from loaded saves (BUG-003)

Once 3.1 and 3.2 land, retest travel *out of a loaded save* specifically — that is BUG-003's exact wording
and it may have its own cause (the save path never populates `Level->TravelInfo`, per phase 2).

---

## Phase 4 — Save package completeness (BUG-005)

Lowest priority of the six; scope it only after 1–3.

`PackageWriter` (`SurrealEngine/Package/PackageWriter.cpp`) writes header, objects, name/export/import
tables. Known gaps visible in the source: `WriteHeader` writes `GenerationCount = 0` and no generations
(line 95), and `Save` renames the previous file to `.old` with the failure swallowed (lines 41-47).
Enumerate what a v436 / v226 save actually needs versus what is written, and fix only what the two target
games read back. Do not chase general-purpose `.u` authoring here — that is a separate goal.

---

## Testing

There is no unit-test suite (per `CLAUDE.md`); verification is by running against a real UE1 install.

Build and deploy per the project convention:

```
cd build && make -j 16 && cp SurrealEngine "Unreal Gold/System/"
```

Manual scenarios, in the order the phases unlock them:

1. **Phase 1** — Unreal Gold, Vortex Rikers: save, load, then (a) survive 60 s without a crash [BUG-001],
   (b) shoot a pane of glass and confirm it breaks, (c) trigger a mover/door and confirm it moves [BUG-002].
2. **Phase 2** — after loading a save, confirm exactly one player pawn exists in `Level->Actors` and that
   HUD, health and inventory reflect the *saved* state, not a fresh spawn.
3. **Phase 3** — walk the first→second Unreal Gold level transition with a Translator in inventory and
   confirm it is still in the item list afterwards [BUG-004]. Then save on level 1, load, travel to
   level 2, and confirm inventory survives [BUG-003].
4. **Regression** — the same save/load/travel loop in UT99 v436, which shares every code path touched here.

VR note: scenarios 1–3 involve the user physically acting in-headset if run under VR, so those runs need the
**240-second timeout** the project mandates. Phases 1–3 are all desktop-testable, though, and should be
verified on the desktop path first — none of this code is VR-specific.

## Risk

The state-frame work in phase 1 is on the **load path for every package**, not only save files — `Load` is
shared. A mistake there breaks map loading outright rather than just save loading. Keep phase 1 as a
self-contained commit that is verified against normal map loading before phase 2 starts.
