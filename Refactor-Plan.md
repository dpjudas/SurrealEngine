# Refactor Plan

A structural review of Surreal Engine (VM, engine core, VR) with a prioritised work list.

This document is deliberately **not** a second bugtracker. `Bugtracker.md` owns the gameplay defect list and
stays authoritative for it. What is collected here is the other half: latent bugs nobody has hit yet,
duplication, and the structural problems that make the gameplay defects expensive to fix. Where the review
turned up a genuine new defect, it is written up here with enough detail to be folded into `Bugtracker.md`
as a numbered entry — see [New defects](#new-defects-for-bugtrackermd).

**Method.** Findings below are either read directly out of the source (file:line given) or measured. The
measured ones came from exporting the real UnrealScript sources out of both target games with
`SurrealDebugger`'s `export scripts` and diffing every `native(N)` declaration against every
`RegisterVMNativeFunc_*` call in the engine — 230 indexed natives on UT99 v436, 145 on Unreal Gold v226.
That method is described in [RP-2](#rp-2--vm-conformance) and is worth keeping as a repeatable check.

Per `Docs/Status.md`, the supported targets are **UT99 v436** and **Unreal Gold v226**. Everything below is
scoped to those unless it says otherwise.

---

## Severity scale

Reused from `Bugtracker.md` so the two lists can be read together.

| | Meaning |
| --- | --- |
| **S1 Blocker** | Crashes, loses progress, or makes a supported game unfinishable. |
| **S2 Major** | Core behaviour is wrong; playable but visibly broken. |
| **S3 Moderate** | Noticeable wrongness that does not block play. |
| **S4 Minor** | Cosmetic, convenience, or maintainability only. |

"Latent" means the defect is real but no known input reaches it today. Latent defects are still worth
fixing when they sit under code you are about to change — WP-1's BUG-007 and BUG-009 were both latent until
the save/load work walked into them.

---

## Work packages, in execution order

| RP | Theme | Severity | Why this order |
| --- | --- | --- | --- |
| [RP-1](#rp-1--object-lifetime-and-the-dead-garbage-collector) | Object lifetime / the dead GC | S1 | Unbounded leak across every map load. Everything else is easier once memory is bounded. |
| [RP-2](#rp-2--vm-conformance) | VM conformance | S2 | Two confirmed script-visible defects, cheap fixes, and a repeatable conformance check. |
| [RP-3](#rp-3--vm-robustness-and-error-handling) | VM robustness | S2 | Several paths turn a script error into UB or a hang instead of a diagnosable failure. |
| [RP-4](#rp-4--engine-god-object-and-the-main-loop) | `Engine` god object + main loop | S3 | Enables RP-5/RP-6 and removes a class of "who owns this" bug. |
| [RP-5](#rp-5--physics-duplication) | Physics duplication | S3 | Three near-identical movement ticks; the divergences between them are where BUG-025 lived. |
| [RP-6](#rp-6--render-and-platform-duplication) | Render / platform duplication | S4 | **Deferred by decision, 2026-07-21** — much of it is editor-side, and the editor is not finished. Revisit once it is. |
| [RP-7](#rp-7--tooling-and-build-hygiene) | Tooling and build hygiene | S4 | No warnings were enabled at all. **Warnings landed 2026-07-21**; the triage is still open. |
| [RP-8](#rp-8--files-to-split-and-logic-that-lives-in-the-wrong-place) | File splits and misplaced logic | S4 | Where the structure is wrong rather than buggy. Behaviour-preserving; do it under whichever work package is already in the file. |

---

## RP-1 — Object lifetime and the dead garbage collector

**Severity S1.** The garbage collector does not work, and nothing calls it.

### What is actually there

Three independent facts, each verifiable in isolation:

1. **`GC::Collect()` has no call sites.** `grep -rn "GC::Collect"` over the whole tree returns only its own
   definition (`GC/GC.cpp:68`) and a comment. `GC::GetStats()` *is* called — from `RenderCanvas.cpp:653`, to
   draw an object count on the debug overlay. So the engine reports its heap and never sweeps it.

2. **The mark phase would not work if it were called.** `GC/GC.cpp:87-95`:

   ```cpp
   GCAllocation* GC::Mark(GCAllocation* marklist)
   {
       GCAllocation* marklistout = nullptr;
       for (GCAllocation* allocation = marklist; allocation != nullptr; allocation = allocation->marklistNext)
       {
           allocation->object()->Mark(marklistout);   // <- return value discarded
       }
       return marklistout;
   }
   ```

   `GCObject::Mark` is declared `GCAllocation* Mark(GCAllocation*)` and every implementation returns the
   extended marklist (`GC::MarkObject` in `GC.h:129-142` is a pure function that returns the new head — it
   cannot extend the caller's list in place). Discarding the result means `marklistout` stays null, so
   `GC::Mark` always returns null and `GC::Collect`'s `while (marklist)` loop terminates after one pass.
   Tracing never gets past the roots' immediate children.

3. **`UObject::Mark` is a stub.** `UObject/UObject.cpp:565-570` — the entire body is commented out and it
   returns `marklist` unchanged. So even a correct trace would reach no `UObject` references at all.

Any one of these makes the GC inert; all three are present.

### The consequence

`Engine::UnloadMap` (`Engine.cpp:629-641`) nulls its pointers and calls
`PackageManager::UnloadPackage`, which (`Package/PackageManager.cpp:187-202`) **only evicts the package
from the open-stream cache**. It frees no objects. With the GC never running, every actor, mesh, texture and
`Bytecode` tree of every level ever loaded stays resident for the process lifetime.

This is the structural explanation for a long session getting heavier the more you travel, and it is
directly in the path of the WP-1 work: `LoadMap` also leaks a `GC::Alloc<UStringProperty>` per call
(`Engine.cpp:721`) and `Engine::Run` leaks three more per process (`Engine.cpp:138-140`).

### Plan

Do **not** start by fixing the GC. Start by measuring, per the WP-3 lesson:

1. **Measure first.** Print `GC::GetStats()` on every `LoadMap`/`UnloadMap` boundary and travel a Unreal Gold
   chain of four or five levels. Record objects and megabytes per transition. This gives the before/after
   number that every later step is judged against, and it settles how urgent this actually is.
2. **Decide the model deliberately.** There are two honest options and they should not be mixed:
   - *Make the GC work* — fix `GC::Mark` to accumulate (`marklistout = allocation->object()->Mark(marklistout);`),
     implement `UObject::Mark` to walk `Class->Properties` over `PropertyData` (the commented-out code is the
     sketch), and call `Collect()` at a safe point — end of `LoadMap`, never mid-tick.
   - *Delete the GC* — if UE1's own lifetime model (packages own their objects; unloading a package destroys
     them) is a better fit, then `UnloadPackage` should destroy the package's exports and `GC/` should go.
     This is arguably closer to what the original engine did.

   Option 1 is the smaller change and keeps `GCRoot` working for the transient allocations
   (`VRHands::Tick` uses one per frame). Recommend option 1.
3. **The blocker for option 1 is `PropertyDataBlock::Reset`** (`UObject.cpp:574-585`), whose body is also
   commented out with `// To do: this crashes as the class might have been destroyed first`. A working
   sweep needs a destruction order that does not depend on `Class` still being alive — either mark `UClass`
   as reachable from every instance (which `UObject::Mark` should do anyway), or capture the property list
   at construction. Solve this before enabling the sweep, not after.
4. **Land the leaks that do not need the GC first**, as they are independent and safe: hoist the three
   `Engine::Run` scratch properties into members, and make the `LoadMap` string property a static or a
   member rather than a per-load allocation.

**Verification.** The transition-boundary numbers from step 1 must stop growing. Then a five-level Unreal
Gold travel chain, a save/load cycle on each, and the UT99 regression pass from WP-1.

---

## RP-2 — VM conformance

**Severity S2.** Two confirmed defects, plus a conformance harness that should be kept.

**The full measurement — counts, per-class stub lists for both games, and the method to re-run it — is in
[`Docs/VMConformance.md`](Docs/VMConformance.md).** This section is the summary and the resulting work.

### How this was measured (keep this)

```
# export the real script sources out of both games
SurrealDebugger <gameroot>   ->   export scripts <PackageName>

# diff every native(N) declaration against every RegisterVMNativeFunc_*(..., N)
```

Result across UT99 v436 (230 indexed natives declared) and Unreal Gold v226 (145):

| | UT99 | Gold |
| --- | --- | --- |
| indexed natives declared by scripts | 230 | 145 |
| declared but **not registered** by the engine | **1** | **1** |
| registered but stubbed to `LogUnimplemented` | 38 | — |

So the native coverage is genuinely near-complete, and the "missing natives" impression from
`Docs/Status.md` is mostly about the *stubbed* set, not an unregistered set. That is worth correcting in the
docs — it changes how BUG-060 (AI natives) should be scoped.

**This diff should become a commandlet.** `SurrealDebugger` already loads every package and already knows
every registered handler; a `natives check` command that prints the unregistered and stubbed sets for the
loaded game would make this a one-line regression check instead of a scripted export. That is the single
highest-leverage tooling item in this document.

### RP-2.1 — String `>` is unregistered on both target games (S2, latent)

`Native/NObject.cpp:98`:

```cpp
RegisterVMNativeFunc_3("Object", "Greater_StrStr", &NObject::Greater_StrStr, 1186);
```

Both games declare it at **116** (`Core/Classes/Object.uc:243`, identical in UT99 and Gold):

```
native(116) static final operator(24) bool   >  ( string A, string B );
```

Its four siblings are all correct — `Less_StrStr` 115, `LessEqual_StrStr` 120, `GreaterEqual_StrStr` 121,
`EqualEqual_StrStr` 122. Index 1186 is a Deus Ex / 227-range index, so this reads as a copy-paste from the
Deus Ex table. Any script evaluating `StrA > StrB` on UT99 or Unreal Gold reaches
`Frame::CallNative`'s "Unknown native function" throw. The implementation itself
(`NObject.cpp:929-932`) is correct and already registered under the old-version branch at 198
(`NObject.cpp:286`) — only the modern-version registration has the wrong number.

**Fix:** register 116, and keep 1186 for Deus Ex. One line. This is the only unregistered index in either
game, which is a good sign for the rest of the table.

### RP-2.2 — `Level.Year` / `Level.Month` are off by 1900 and by 1 (S3, user-visible)

`Engine.cpp:167-174`:

```cpp
LevelInfo->Year()  = timedesc->tm_year;   // tm_year is years since 1900
LevelInfo->Month() = timedesc->tm_mon;    // tm_mon is 0-11
```

UE1's `LevelInfo.Year` is the calendar year and `LevelInfo.Month` is 1-12. Both games rely on it:

- `UnrealShare/Classes/UnrealSaveMenu.uc:27` — `MonthNames[Level.Month - 1]`. In January this indexes
  **`MonthNames[-1]`**. This is in Unreal Gold's *save game menu*, i.e. squarely inside the area WP-1 has
  been working on.
- `Botpack/Classes/TournamentGameInfo.uc:366-376` and `Engine/Classes/StatLog.uc:96` build a date string
  from `Month`/`Day`/`Year`, which currently renders as e.g. `6/21/126`.

`Millisecond` is also hardcoded to 0 with a comment saying there is no `tm` equivalent — there isn't, but
`std::chrono` has one.

**Fix:** `tm_year + 1900`, `tm_mon + 1`, and fill `Millisecond` from a `std::chrono::system_clock` sub-second
remainder. Also replace `std::localtime` (not thread-safe, returns a shared static) with `localtime_r` /
`localtime_s`.

### RP-2.3 — Conversion operators that do not match UE1 (S3/S4, mostly latent)

All in `VM/ExpressionEvaluator.cpp`:

| Line | Expression | Problem |
| --- | --- | --- |
| 516-544 | `StringToVector`, `StringToRotator` | Parses bare `"1,2,3"`. UE1 formats vectors as `X=1.0,Y=2.0,Z=3.0`, so `atof("X=1.0")` returns 0 and every component comes out zero. |
| 571-574 | `BoolToString` | `std::to_string(bool)` yields `"1"`/`"0"`; UnrealScript's `string(bool)` is `"True"`/`"False"`. Note the *property* export path may already be correct — check `UProperty::PrintValue` before changing both. |
| 581-585 | `ObjectToString` | Returns `Package.Name`; UE1 returns the object name alone. |
| 598-602 | `RotatorToString` | Masks each component with `0xffff`, so negative angles print as large positives. |
| 358-364 | `DynamicCast` | Tests `value->IsA(expr->Class->Name)` — **by name**. Two classes with the same name in different packages compare equal. `MetaCast` (210-226) correctly walks the pointer chain; `DynamicCast` should too. |
| 436-439, 466-469 | `IntToByte`, `FloatToByte` | No clamp/mask before narrowing to `uint8_t`; a negative float is UB rather than UE1's wrap. |

These are cheap and independent. Do them as one commit with a short note each, not one commit per line.

### RP-2.4 — Expression kinds that throw (S3, verify reachability first)

Four evaluator cases call `Frame::ThrowException` unconditionally:

- `LabelTableExpression` (`:104-108`) — **the one to check.** Label tables are the last statement of every
  state's bytecode (that is exactly what `Bytecode::FindLabelIndex` relies on, `Bytecode.h:31-43`). Normally
  a `Stop` precedes them, but a state body that falls off its end would execute the table and throw. Worth a
  disassembly sweep over both games' state code to see whether any state can fall through.
- `NativeParmExpression` (`:333-336`), `ConstructExpression` (`:682-685`), `Unknown0x46Expression`
  (`:481-484`) — believed 227/UE2-only. Confirm with the same sweep and, if they are unreachable on the two
  targets, say so in the message so the next reader does not have to re-derive it.

The `Unknown0x15Expression` case (`:228-233`) silently returns `StatementResult::Stop` with a comment about
Klingon Honor Guard. That is a guess baked into the hot path; it deserves the same treatment.

---

## RP-3 — VM robustness and error handling

**Severity S2.** The VM's failure paths are the weakest part of it. None of these is a wrong answer — they
are cases where a script bug becomes undefined behaviour, a hang, or silence instead of a diagnosable error.

### RP-3.1 — `ThrowException` does not throw (S2)

`Frame::ThrowException` (`Frame.cpp:131-139`) sets `ExceptionText` and calls `Break()`. `Break()`
(`:86-106`) only converts that into a real `Exception::Throw` **when no debugger is attached**. With
`SurrealDebugger` attached, `RunDebugger()` runs and then control *returns to the caller*, which continues
as if nothing happened. Every call site is written as if it does not return.

The sharpest case is `Frame::Run` (`Frame.cpp:478-479`):

```cpp
if (StatementIndex >= Func->Code->Statements.size())
    ThrowException("Unexpected end of code statements");

size_t curStatementIndex = StatementIndex;
StatementIndex++;
StepExpression = Func->Code->Statements[curStatementIndex];   // out of bounds
```

Under the debugger this is an out-of-bounds read on the very condition it just diagnosed.

**Fix:** make `ThrowException` `[[noreturn]]` and unwind out of the frame properly, with the debugger break
happening *before* the unwind. This is the fix that makes the rest of RP-3 straightforward.

### RP-3.2 — The runaway-loop guard does not stop anything (S2)

`Frame.cpp:474-491`:

```cpp
const int maxInstructions = 500'000;
...
if (instructionsRetired >= maxInstructions)
{
    LogMessage("Too many VM instructions executed in a single tick");
    Break();
}
```

The check is inside the loop and neither breaks nor returns. Once the budget is exceeded, every subsequent
iteration logs the same line again and the loop keeps running — a runaway script produces an unbounded log
and a hang instead of the intended abort. (Without a debugger, `Break()` also does nothing here, because
`ExceptionText` is empty.)

**Fix:** abort the frame — the whole point of the budget. UE1's own behaviour is a "runaway loop" script
error, which is the right model.

### RP-3.3 — No call-depth limit (S2, latent)

`Frame::Callstack` (`Frame.h:69`, pushed by `ActiveCallStackFrame`, `:109-113`) is unbounded, and
`ExpressionEvaluator::Eval` recurses on the C++ stack for every nested expression. Infinitely recursive
script — trivially writable, and reachable from a mod or a mis-set state — overflows the C++ stack and
crashes the process with no diagnostic. The bytecode *parser* already guards itself at depth 64
(`Bytecode.cpp:17-18`); the evaluator has no equivalent.

**Fix:** a depth limit on `Frame::Callstack` (UE1 used 250) that raises a script error carrying
`GetCallstack()`.

### RP-3.4 — Native calls swallow every exception (S2)

`Frame::CallNative` wraps each dispatch in `catch (const std::exception&)` + `catch (...)`
(`Frame.cpp:327-340` and again at `:355-367` — the two branches are otherwise identical and should be one
helper). Both log and return `NothingValue()`. That converts a real engine fault inside a native — a bad
cast, a failed allocation, a package read error — into a silently wrong script value, and it also swallows
the `Exception::Throw` that `Break()` uses to report script errors. The `catch (...)` in particular will
eat anything.

**Fix:** narrow to the exception type natives are actually expected to raise, and let everything else
propagate. This is a behaviour change and wants its own commit and its own regression pass.

### RP-3.5 — Smaller VM items

- `Bytecode::FindLabelIndex` (`Bytecode.h:31-43`) calls `Statements.back()` without checking for an empty
  `Statements`, and uses `dynamic_cast` on every state transition. Both are one-line fixes (an `Expression`
  kind tag is already available for the visitor).
- `Frame::Run` returns on `Stop`/`Return` without unwinding `Iterators`. An iterator left on the frame is
  then popped by whatever runs next.
- `ExpressionEvaluator::Eval` (`:16-22`) scans the whole `Frame::Breakpoints` array for **every expression
  node evaluated**, and writes the global `Frame::StepExpression` twice per node. Cheap when the array is
  empty, but this is the hottest loop in the engine; gate the whole block on `Breakpoints.empty()`.
- Every piece of VM state is a mutable global (`Frame.h:67-82`: `Callstack`, `RunState`, `StepFrame`,
  `ExceptionText`, `CreatedIterator`, `Breakpoints`). This forecloses ever running script off the main
  thread and makes the debugger's state and the VM's state the same state. Not urgent; worth recording as
  the reason the VM cannot be tested in isolation today.
- `Frame::TraceCall` (`Frame.cpp:378-430`) is 50 lines behind `#if 0` with a `// To do: create a commandlet
  that lets us do this`. Either build the commandlet or delete it — dead code that documents a wish is worse
  than an issue.

---

## RP-4 — `Engine` god object and the main loop

**Severity S3.** Nothing here is a bug today. It is the reason several of the bugs above were hard to find.

### The shape of the problem

`Engine.h` declares ~115 members in a single `public:` section, covering: the object graph
(`LevelInfo`, `Level`, `GameInfo`, `EntryLevel*`), the render/audio/input subsystems, the VR subsystem and
its three helpers, camera state, keybindings and input-alias maps, travel state, save-slot state, Deus Ex
specifics (`dxRootWindow`, `dxSaveInfo`, `dxConMissionList`, `DeusExLevelInfo`, `deusExPackage`), editor
flags, and three fields explicitly labelled `// Collision debug`. It is also the `GameWindowHost`. A single
global `Engine* engine` is dereferenced **804 times across 54 files**.

`Engine::Run` (`Engine.cpp:87-317`) is 230 lines that do: settings load, VR construction, window and audio
init, entry-map load, then a loop that advances two clocks, stamps the wall-clock date, ticks input, ticks
two levels, ticks the Deus Ex window tree, calls `PlayerCalcView` through a hand-rolled property-argument
dance, applies the VR aim/view split, ticks VR hands, updates audio, renders, handles a save request, and
then handles five different travel routes — four of which end in the same `LoadMap(...); LoginPlayer();`
pair.

### Plan

Sequenced so each step is independently shippable and testable.

1. **Extract the loop body into named phases** on `Engine` itself, no new types:
   `AdvanceTime()`, `TickInput()`, `TickLevels()`, `UpdateCamera()`, `RenderFrame()`, `ProcessPendingSave()`,
   `ProcessPendingTravel()`. Pure motion, no behaviour change; this alone makes the VR insertion points and
   the travel branches legible.
2. **Collapse the travel routes.** The five branches at `Engine.cpp:241-298` differ only in the URL they
   build, the travel info they pass, and the `TravelType` they declare. One `PendingTravel` struct with those
   three fields, and one place that consumes it. The comment at `:246-252` — added in WP-1 phase 3 precisely
   because the branches were inheriting each other's state — is describing this problem.
3. **Hoist the `PlayerCalcView` argument scaffolding.** The three `GC::Alloc`'d scratch properties
   (`:138-140`) mutated per frame (`:201-202`) exist because `CallEvent` cannot pass out-params. Fix
   `CallEvent` (the `// To do: improve CallEvent so parameter passing isn't this painful` at `:197` is the
   same complaint) rather than keeping the scaffolding.
4. **Split the state out of `Engine` into owned components**, in this order (easiest and least entangled
   first):
   - `DeusExState` — five members and a handful of `IsDeusEx()` branches, entirely separable.
   - `InputState` — `keybindings`, `inputAliases`, `activeInputButtons`, `activeInputAxes`, `MouseMoveX/Y`,
     and the `Key`/`InputEvent`/`InputCommand`/`ExecCommand` group.
   - `CameraState` — the four camera fields, which are what the VR aim/view split writes through.
   - The three `// Collision debug` fields, which should live behind the same debug-overlay switch that
     reads them.
5. **Leave the global `engine` pointer alone for now.** Removing it is an 800-site change that would swamp
   the review of everything above. Revisit once the components exist.

**Verification.** Each step is behaviour-preserving; the check is the existing manual pass — boot both
games, travel a level, save and load, and run one VR session.

---

## RP-5 — Physics duplication

**Severity S3.** `UObject/UActor.cpp` is 4650 lines and holds the largest *semantic* duplication in the tree.

`TickSwimming` (`:805-923`) and `TickFlying` (`:925-1017`) are the same function. Both: reject non-pawns,
handle `ZoneNumber == 0`, save `OldLocation`, clamp acceleration to `AccelRate`, apply the same
`ZoneFluidFriction` blend in both the accelerating and decelerating branch, integrate, clamp to a max
speed, then run the identical five-iteration `TryMove` + `HitWall` + slide loop, then recompute velocity
from displacement. They differ in exactly four places: the `0.3f` factor on `AccelRate`, `WaterSpeed` vs
`AirSpeed`, swimming's pushable-decoration case, and swimming's leaving-the-water push. The same slide loop
appears a third time in `TickFalling` (`:679-803`).

This is not a hypothetical cost. **BUG-025 was two bugs of exactly this shape** — an `isMoving` test written
with `&&` across the velocity components, and a copy-pasted `Velocity().z = 0` that had no business being in
the flying tick. Both survived because the two functions had to be read side by side to notice.

### Plan

1. Extract the slide loop first — `SlideMove(vec3 velocity, float elapsed, int maxIterations)` returning the
   final hit — and use it from swimming, flying and falling. This is the piece that is genuinely identical
   and where a divergence is most dangerous.
2. Extract the acceleration/friction/max-speed integration as `ApplyFluidAcceleration(accelRate, maxSpeed,
   friction, elapsed)`; swimming and flying then differ only in their two arguments plus their own tail.
3. Keep swimming's pushable-decoration branch and water-exit push where they are — they are genuinely
   swimming-specific, and folding them into a shared function behind a flag would recreate the problem in a
   worse form.
4. While in here, one behavioural question worth resolving rather than preserving: the slide loop never
   recomputes `vel` after a `HitWall` handler has modified `Velocity()`, so a script that changes velocity
   on impact is ignored for the rest of the tick. Check against UE1 before changing it.

**Verification.** BUG-025's own repro (swimming straight up and straight down), plus the WP-3 repros:
walking onto another actor's cylinder, mover approach angles, and the DM-Conveyor catwalk.

---

## RP-6 — Render and platform duplication

**Severity S4. Deferred by decision on 2026-07-21** — a good part of what is listed below is editor-side or
sits under code the unfinished editor will reshape, so consolidating it now would mean doing it twice.
Recorded in full so the measurement does not have to be repeated when the editor lands.

No behaviour change; this is the cheapest large win in the tree once it is worth taking.

A 12-line-window duplicate scan over the whole source tree finds 1043 distinct duplicated blocks. Excluding
`Math/halffloat.cpp` (a generated table, 928 blocks — ignore it), the concentrations are:

**`Render/VisibleMesh.cpp` (346 blocks, 1041 lines).** Every draw path has a forked Deus Ex twin:
`DrawMesh`/`DrawMeshDX`, `DrawLodMesh`/`DrawLodMeshDX`, `DrawLodMeshFace`/`DrawLodMeshFaceDX`. Roughly 440
lines are a copy with the Deus Ex blend handling threaded through. The two versions have already drifted.
Merge them behind the blend parameter that `DrawLodMeshFaceDX` already takes (`BlendInfo*`, `int`), with a
null/zero meaning the non-DX path.

**`RenderDevice/*/…TextureUploader.cpp` (118 blocks each).** The D3D11 and Vulkan uploaders are the same
format-conversion logic twice; the per-format decode is API-independent and should live once above the
backend split.

**`RenderDevice/*/…FileResource.cpp` (471 + 366 lines).** Both are one giant `if (filename == "...") return
R"(...)"` chain with every shader inlined as a raw string literal — HLSL in one, GLSL in the other. The
author's own comment leads both files: `// I probably should find a less brain dead way of doing this. :)`.
Move the shaders to real files and load them from `SurrealEngine.pk3`, which is where `CLAUDE.md` already
claims they live. (They are not in it — the archive holds only icons and fonts. That doc line should be
corrected either way.)

**`MainGame.cpp` / `MainEditor.cpp` (91 lines each).** Byte-identical except `GameApp` vs `EditorApp` on
three lines. Collapse to a shared template or a single `.cpp` with the app type as a parameter.

**`VM/NativeFunc.h:28-317`.** Thirty-two near-identical registration templates — `RegisterVMNativeFunc_0`
through `_15`, twice (with and without a `UObject* self` first parameter). A variadic template with an
index-sequence expansion replaces all of them with about twenty lines, and makes the arity mismatch between
a handler and its script declaration a compile error instead of an argument-array overread.

---

## RP-7 — Tooling and build hygiene

**Severity S4**, but this is where the leverage is.

1. **~~No warnings are enabled.~~ DONE 2026-07-21 — the triage is the remaining work.** `CMakeLists.txt`
   previously set no `-Wall`, no `-Wextra`, no `/W4`; the only compile flag was MSVC's `/MP`. There is now an
   `ENABLE_WARNINGS` option (default ON) applying `-Wall -Wextra` (`/W4` on MSVC) to `SurrealCommon` and the
   three executables, with `Thirdparty/` and `SurrealVideo/` silenced per-source. Deliberately no `-Werror`,
   so it cannot break anyone's build.

   A clean g++ Release build emits **~510 warnings** after three style suppressions
   (`-Wno-unused-parameter`, `-Wno-missing-field-initializers`, `-Wno-unused-function`; the last is 326
   warnings on its own, all from `static` enum-flag operator overloads in headers — the real fix there is
   `inline` rather than `static`, which is a mechanical sweep worth doing). What remains, untriaged:

   | Warning | Count | Where |
   | --- | --- | --- |
   | `-Wclass-memaccess` | ~180 | `UObject/PropertyOffsets.cpp` — memory ops over non-trivial types, in the property layer, which is precisely where that is worth looking at. |
   | `-Wsign-compare` | ~157 | Spread; `VM/ExpressionValue.h` is the concentration. |
   | `-Wreorder` | ~60 | `UObject/USubsystem.h` — member init order vs declaration order. |
   | `-Wunused-variable` | ~46 | Spread. |
   | `-Wswitch` | 13 | Four `VisibleMesh.cpp` draw-style switches do not handle `STY_None`, `STY_Normal` or `STY_AlphaBlend`. Worth checking against WP-6's render defects before dismissing. |
   | `-Wuninitialized` | 11 | All from one site, `UObject/UObject.h:139`. |

   Triage these as separate commits per cluster, not one sweep — `-Wclass-memaccess` in the property layer
   in particular is a correctness question, not a cleanup.
2. **`natives check` commandlet** — described in [RP-2](#rp-2--vm-conformance). Highest-value single tool.
3. **Make the commandlets survive bad input.** Both failures below stopped a full-tree export dead:
   - `export scripts` with **two or more** package names is an infinite loop.
     `ExportCommandlet.cpp:83-91`: `sep` is computed once and never updated, so
     `while (sep != std::string::npos)` spins forever re-pushing the first name. One package works by
     accident (`sep` is `npos` immediately).
   - One unreadable package aborts the whole run. `export scripts` with no arguments died on
     `Could not find package AchtungDieKurve` (UT99) and on
     `ObjectStream::ReadString: Invalid size in Crashsite2` (Unreal Gold) — see
     [BUG-071](#new-defects-for-bugtrackermd). Per-package error isolation would have let both runs finish
     and *report* the failures, which is exactly what a diagnostic tool is for.
   - A missing `Maps/` directory throws out of a `directory_iterator` rather than being treated as "no maps".
4. **`build-asan/` already exists** — the sanitizer build is clearly in use. Promote it to a documented CMake
   preset so it is a one-liner rather than tribal knowledge, and note it in `Docs/Building.md`.
5. **Doc drift to fix while here:** the `SurrealEngine.pk3` shader claim in `CLAUDE.md`, and
   `Docs/Status.md`'s framing of missing natives (RP-2 shows the gap is stubs, not registration).

---

## Defects filed from this review

All of these are now in [`Bugtracker.md`](Bugtracker.md), which is authoritative for their status. Listed
here only as an index from work package to bug ID.

| ID | Sev | WP | Defect | Section |
| --- | --- | --- | --- | --- |
| BUG-045 | S2 | WP-5 | The garbage collector is dead three ways and nothing calls it; every `UObject` of every level leaks for the process lifetime. | [RP-1](#rp-1--object-lifetime-and-the-dead-garbage-collector) |
| BUG-046 | S2 | WP-5 | `Maps/UPak/Crashsite2.unr` (Return to Na Pali) fails to load — `ObjectStream::ReadString: Invalid size`. | [RP-7](#rp-7--tooling-and-build-hygiene) |
| BUG-070 | S2 | WP-9 | String `>` is registered at native index 1186 instead of 116, on both target games. | [RP-2.1](#rp-21--string--is-unregistered-on-both-target-games-s2-latent) |
| BUG-072 | S3 | WP-9 | `Level.Year` is off by 1900 and `Level.Month` is 0-based; Unreal Gold's save menu indexes `MonthNames[-1]` in January. | [RP-2.2](#rp-22--levelyear--levelmonth-are-off-by-1900-and-by-1-s3-user-visible) |
| BUG-073 | S2 | WP-9 | The runaway-instruction guard neither aborts the frame nor exits the loop. | [RP-3.2](#rp-32--the-runaway-loop-guard-does-not-stop-anything-s2) |
| BUG-074 | S2 | WP-9 | `Frame::ThrowException` returns to its caller under the debugger; `Frame::Run` then reads out of bounds. **Fix first.** | [RP-3.1](#rp-31--throwexception-does-not-throw-s2) |
| BUG-075 | S2 | WP-9 | No call-depth limit — script recursion is a silent stack-overflow crash. | [RP-3.3](#rp-33--no-call-depth-limit-s2-latent) |
| BUG-076 | S3 | WP-9 | `StringToVector`/`StringToRotator` parse the wrong format, so every component comes out 0. | [RP-2.3](#rp-23--conversion-operators-that-do-not-match-ue1-s3s4-mostly-latent) |
| BUG-077 | S4 | WP-9 | `string(bool)`, `string(object)` and `string(rotator)` all disagree with UE1. | [RP-2.3](#rp-23--conversion-operators-that-do-not-match-ue1-s3s4-mostly-latent) |
| BUG-078 | S3 | WP-9 | `DynamicCast` compares class names rather than class identity. | [RP-2.3](#rp-23--conversion-operators-that-do-not-match-ue1-s3s4-mostly-latent) |
| BUG-079 | S4 | WP-10 | `export scripts` loops forever on two or more packages; one bad package aborts the whole run. | [RP-7](#rp-7--tooling-and-build-hygiene) |
| BUG-081 | S4 | WP-10 | No repeatable native-coverage check — needs a `natives check` commandlet. | [RP-2](#rp-2--vm-conformance) |

The full conformance measurement behind BUG-070 — counts, per-class stub lists for both games, and the
method to re-run it — is in [`Docs/VMConformance.md`](Docs/VMConformance.md).

---

## RP-8 — Files to split, and logic that lives in the wrong place

**Severity S4** on its own, but this is the answer to "what is structurally wrong" rather than "what is
buggy". Nothing here changes behaviour; all of it changes how expensive the next bug is to find.

### The four files that should be split

| File | Lines | What is actually in it |
| --- | --- | --- |
| `UObject/UActor.cpp` | 4650 | See below — three unrelated subsystems. |
| `UObject/UActor.h` | 2586 | **57 class definitions.** The entire UE1 actor hierarchy in one header. |
| `UObject/UWindow.cpp` | 5130 | 33 Deus Ex UI widget classes. |
| `UObject/PropertyOffsets.{h,cpp}` | 4337 + 5633 | ~10,000 lines of generated table. |

**`UObject/UActor.h` — 57 classes in one header.** `UActor`, `ULight`, `UInventory`, `UWeapon`, the eight
`UNavigationPoint` subclasses, `UDecoration`, `UProjectile`, `UTrigger`, `UHUD`, `UMenu`, the `UInfo` tree
(`UGameInfo`, `UZoneInfo`, `ULevelInfo`, `UStatLog`, the replication infos), `UBrush`/`UMover`, the `UPawn`
tree, *and* the Deus Ex classes (`UDeusExPlayer` at 2334, `UScriptedPawn`, `UDeusExDecoration`) and the UPak
iterators. Everything that touches any actor includes all of it. Split along the inheritance tree the file
already groups by — `UActorBase.h`, `UInventory.h`, `UNavigation.h`, `UInfo.h`, `UBrush.h`, `UPawn.h` — which
is a mechanical cut, not a redesign.

**`UObject/UWindow.{h,cpp}` — 6600 lines of Deus Ex.** This is the Deus Ex UI toolkit (`UListWindow`,
`UComputerWindow`, `UScaleWindow`, `UEditWindow`, `UGC`…) sitting in the core object-model directory, and
Deus Ex is **WP-8 parked**. It should be its own directory, isolated behind one boundary — see below.

**`UObject/PropertyOffsets.{h,cpp}` — generated code, checked in, then hand-edited.** These are produced by
`NativeCppGenerator` (`native oldgenerator` in `SurrealDebugger`, writing to `Cpp/UObject/PropertyOffsets.cpp`),
but the last three commits touching them are manual property additions. There are also **two** generators —
`NativeCppGenerator` ("old") and `NativeCppUpdater` (`native update`) — with nothing saying which is
current. This needs one decision: either regenerate as a build step and stop hand-editing, or declare them
hand-maintained and delete the dead generator. Note the `-Wclass-memaccess` cluster (~180 warnings) lives
here, so whichever way this goes, it should go before that triage.

### Logic that belongs somewhere else

Ordered by how much it would move.

1. **Physics is inside the object model.** `UActor.cpp` holds `TickWalking`, `TickFalling`, `TickSwimming`,
   `TickFlying`, `TickRotating`, `TickProjectile`, `TickRolling`, `TickInterpolating`, `TickMovingBrush`,
   `TickSpider`, `TickTrailer`, `PhysLanded`, `TurnBasedActors` — roughly **900 lines of movement
   integration and collision response** in a file whose job is the UE1 object model. It is the natural peer
   of `Collision/`, which it calls into constantly, and it is where RP-5's duplication lives. A `Physics/`
   module taking an actor and a timestep is the single highest-value move in this section.

2. **Animation is in the same file.** `PlayAnim`, `LoopAnim`, `TweenAnim`, `PlayBlendAnim`, `FinishAnim`,
   `TickAnimation`, `TickBlendAnimation`, `SetTweenFromAnimFrame` — another ~400 lines, unrelated to
   physics and unrelated to the object model. Belongs with `UMesh` or in its own module.

   Those two moves alone take `UActor.cpp` from 4650 lines to roughly 3300, without touching a line of logic.

3. **A console command interpreter lives in `Engine`.** `Engine::ConsoleCommand` (`Engine.cpp:1111-1450`),
   `ExecCommand`, `GetArgs`, `GetSubcommands` — ~370 lines of command parsing and dispatch inside the class
   that owns the main loop.

4. **Video playback sequencing lives in `Engine`.** `Engine::PlayAVI` and `Engine::PlayVideo`
   (`Engine.cpp:319-524`), ~200 lines including a Klingon-Honor-Guard-specific intro path, in `Engine`
   while a `Video/` module already exists.

5. **Input mapping lives in `Engine`.** `LoadKeybindings`, `Key`, `InputEvent`, `InputCommand`,
   `UpdateInput`, the `keynames[256]` table and four `std::map` members. This is item 4 of RP-4 and is
   listed again here because it is the clearest single-responsibility violation in the file.

6. **The debugger lives inside the VM's hot path.** `Frame` owns `Breakpoints`, `RunState`, `StepFrame`,
   `StepExpression` and `RunDebugger` as statics (`Frame.h:67-82`), and `ExpressionEvaluator::Eval` consults
   them on **every expression node evaluated**. The debugger is a legitimate feature, but it should hang off
   an interface the VM calls, not be the VM's own state — that is also what makes the VM impossible to test
   in isolation today (RP-3.5).

7. **VR drawing lives in the canvas.** `Render/RenderCanvas.cpp` mixes `DrawVRHudPlane`, `DrawVRWheel`,
   `DrawVRActiveItem`, `DrawVRCrosshair`, `DrawVRAimLaser` with the ordinary 2D canvas *and* the debug
   overlay that prints `GC::GetStats()`. The VR half has a natural home in `VR/`. Low priority — the
   coupling is honest and documented in `Docs/VR.md`.

### The Deus Ex boundary

Deus Ex is **parked** (WP-8), yet it is the largest single contributor to the size of the core files:
`UWindow.{h,cpp}` is 6600 lines of it, `UActor.h` carries `UDeusExPlayer`/`UScriptedPawn`/`UDeusExDecoration`,
`Engine.h` carries five `dx*` members, and `IsDeusEx()`/`DeusEx` appears in **20 files that are not
Deus-Ex-named**, including `Render/VisibleMesh.cpp`, `Render/VisibleActor.cpp`, `Package/PackageManager.cpp`
and four `Native/N*.cpp` files (35 `IsDeusEx()` call sites in `.cpp` alone).

Pulling it behind one boundary — a `DeusEx/` directory plus a single game-specific hook interface, rather
than `IsDeusEx()` branches scattered through the render and native layers — would shrink the files the
supported games actually use, and would make the parked status real instead of nominal. It is a large
change and should not be started before RP-1 and RP-9 land, but it is the one that would most change how
this codebase reads.

## What this review did not cover

Stated so the gaps are known rather than assumed clean:

- **Collision** (`Collision/`) was not reviewed in depth. WP-3 has been through it recently with
  instrumentation, which is a better basis than a code read.
- **Audio**, **Video**, **Light** and the **Compiler** subtree (3923 lines, an UnrealScript compiler that
  nothing in the game path calls) were only surveyed.
- **VR** (`VR/`, 2320 lines) held up well: the abstract-base/null-object split is honoured, coupling into the
  engine is 54 references across five files, and the invariants in `Docs/VR.md` match the code. The only
  notes are cosmetic — `Engine::Run` guards `vrInput`/`vrHands`/`vrWheel` for null when all three are
  unconditionally constructed a few lines earlier (`Engine.cpp:98-100` vs `:218-224`), and `VRPlayerInput.cpp`
  at 737 lines is the one file there that would benefit from the RP-4 treatment. It is not a refactor
  priority.
- **Correctness of the render output** is out of scope; `Bugtracker.md` WP-6 owns it.

---

## Suggested sequencing

RP-2.1 and RP-2.2 are one-line fixes with immediate user-visible value — do them first, independently of
everything else. RP-1's *measurement* step should start in parallel because it takes a play session to
produce numbers. RP-3.1 (`ThrowException`) unblocks the rest of RP-3 and should land before the other VM
robustness items. RP-4 through RP-7 are behaviour-preserving and can be interleaved with gameplay work.
