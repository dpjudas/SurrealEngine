# UnrealScript VM conformance

A measured comparison of Surreal Engine's UnrealScript VM against the actual scripts shipped by the two
supported games: **UT99 GOTY v436** and **Unreal Gold v226**.

Run date: 2026-07-21. Findings are filed as BUG-070 and BUG-072…078 in [`Bugtracker.md`](../Bugtracker.md)
(WP-9); the structural context is in [`Refactor-Plan.md`](../Refactor-Plan.md) RP-2 and RP-3. This document
holds the raw result and the method, so the audit can be re-run and diffed rather than re-derived.

## Headline

**Native coverage is essentially complete.** Of 230 distinct native indices declared by UT99's scripts and
145 by Unreal Gold's, exactly **one** is unregistered — and it is the same one in both games.

This matters because `Docs/Status.md` reads as though large parts of the native table are absent. They are
not. What is absent is *behaviour* behind about 30 handlers that register fine and then call
`LogUnimplemented`. That distinction changes how BUG-060 (AI behaviour) should be scoped: it is stubs to
fill in, not natives to wire up.

| | UT99 v436 | Unreal Gold v226 |
| --- | --- | --- |
| Script packages scanned | 15 | 9 |
| `.uc` sources exported | 1441 | 703 |
| Distinct `native(N)` indices declared | 230 | 145 |
| Named `native` function/event declarations | 202 | 75 |
| **Indices declared but not registered** | **1** (116) | **1** (116) |
| Handlers registered but fully stubbed | 30 | 5 |
| Handlers registered, one 227-only parameter ignored | 2 | 2 |

Engine side, for scale: 1021 `RegisterVMNativeFunc_*` call sites across the tree.

## The one unregistered native

`Core/Classes/Object.uc:243` — byte-identical in both games:

```
native(116) static final operator(24) bool   >  ( string A, string B );
```

`Native/NObject.cpp:98` registers it at **1186**:

```cpp
RegisterVMNativeFunc_3("Object", "Greater_StrStr", &NObject::Greater_StrStr, 1186);
```

1186 is a Deus Ex / 227-range index, so this reads as a copy-paste from that table. The four sibling
operators are all correct:

| Operator | Declared | Registered | |
| --- | --- | --- | --- |
| `<`  | 115 | 115 (`NObject.cpp:116`) | ok |
| `>`  | **116** | **1186** (`NObject.cpp:98`) | **wrong** |
| `<=` | 120 | 120 (`NObject.cpp:111`) | ok |
| `>=` | 121 | 121 (`NObject.cpp:92`) | ok |
| `==` | 122 | 122 (`NObject.cpp:77`) | ok |

The handler itself (`NObject.cpp:929-932`) is correct and is *also* registered correctly at 198 under the
old-package-version branch (`NObject.cpp:286`) — only the modern-version registration carries the wrong
number. Any script evaluating `StrA > StrB` reaches `Frame::CallNative`'s "Unknown native function" throw.

Latent: no script in either game is known to hit it. Fix is one line — register 116, keep 1186 for Deus Ex.
Filed as **BUG-070**.

## Registered but stubbed

These register successfully and then log `LogUnimplemented`, so script calls return a default value with no
error. Grouped by what the absence actually costs.

### UT99 v436

**Stat logging — 16 handlers, the largest single cluster.**
`StatLog`: `BatchLocal`, `BrowseRelativeLocalURL`, `ExecuteLocalLogBatcher`, `ExecuteSilentLogBatcher`,
`ExecuteWorldLogBatcher`, `GetGMTRef`, `GetMapFileName`, `GetPlayerChecksum`, `InitialCheck`, `LogMutator`.
`StatLogFile`: `CloseLog`, `FileFlush`, `FileLog`, `GetChecksum`, `OpenLog`, `Watermark`.
Only relevant to server-side match logging; harmless single-player.

**Web / network — 11 handlers.**
`WebRequest`: `AddVariable`, `DecodeBase64`, `GetVariable`, `GetVariableCount`, `GetVariableNumber`.
`WebResponse`: `ClearSubst`, `IncludeBinaryFile`, `IncludeUHTM`, `Subst`.
`InternetLink`: `ParseURL`, `Validate`.
Blocked behind the missing networking subsystem anyway (WP-8).

**AI and pawn behaviour — 2 handlers.** `Pawn.FindStairRotation` (returns 0), `Pawn.CheckValidSkinPackage`.
`FindStairRotation` is the one worth attention: it is what lets a bot pitch its view on stairs, which is
plausibly related to **BUG-061** (bots rotating their whole body to look up and down). Not confirmed — check
before acting.

**Asset streaming — 3 handlers.** `Actor.GetCacheEntry`, `Actor.MoveCacheEntry`, `Actor.LinkSkelAnim`.
The cache pair is UE1's package-cache browsing, used by menus; `LinkSkelAnim` is skeletal animation, which
UT99 v436 does not use for anything shipped.

**Miscellaneous — 4.** `Commandlet.Main`, `Console.SaveTimeDemo`, `GameInfo.GetNetworkNumber` (marked
`[U227]`), `Object.ResetConfig`.

Not counted above but present in the engine and worth knowing about: a further ~60 `LogUnimplemented`
handlers exist for classes that **neither target game declares** — Deus Ex (`DumpLocation`, `DebugInfo`,
`ScriptedPawn`, `ConEvent`), Unreal 227 (`Object.Quat*`, `Canvas.Draw*`, `Projector`), and UPak. They are
parked (WP-8) and are not conformance gaps for v436/v226.

### Unreal Gold v226

Five, all also in the UT99 list: `Commandlet.Main`, `InternetLink.ParseURL`, `InternetLink.Validate`,
`Object.ResetConfig`, and `Object.SaveConfig` (partial, below).

### Partial, not stubbed

Two handlers are fully implemented for the target games and only ignore a parameter that does not exist in
v436/v226:

- `Actor.Trace` — `UActor.cpp:1542`, ignores the U227-only `BSPTraceFlags`.
- `Object.SaveConfig` — `NObject.cpp:1430`, ignores the U227k-only `bNoWriteINI`.

Neither is a defect on the supported targets. They appear in a naive `LogUnimplemented` grep, which is why
they are called out here.

## Interpreter gaps (not native functions)

Separate from the native table, four expression kinds in `VM/ExpressionEvaluator.cpp` throw unconditionally.
**Their reachability on the two targets has not been measured** — this needs a disassembly sweep, and until
someone runs it these are open questions rather than findings:

| Expression | Line | Note |
| --- | --- | --- |
| `LabelTableExpression` | 104-108 | **The one that matters.** Label tables are the last statement of every state's bytecode — `Bytecode::FindLabelIndex` (`Bytecode.h:31-43`) depends on exactly that. Normally a `Stop` precedes them, but a state body falling off its end would execute the table and throw. |
| `NativeParmExpression` | 333-336 | Believed 227/UE2-only. |
| `ConstructExpression` | 682-685 | Believed 227/UE2-only. |
| `Unknown0x46Expression` | 481-484 | Unidentified opcode. |

`Unknown0x15Expression` (`:228-233`) does not throw — it silently returns `StatementResult::Stop` on the
strength of a comment about Klingon Honor Guard. That is a guess baked into the hot path and deserves the
same sweep.

Short-circuit evaluation *is* handled correctly: `ExpressionEvaluator::Call` (`:687-705`) special-cases
native 130 (`&&`) and 132 (`||`) so the right-hand `skip` operand is not evaluated eagerly.

Conversion operators that compute the wrong answer are filed separately as BUG-076, BUG-077 and BUG-078.

## Method — how to re-run this

Two steps. The second should become a commandlet (**BUG-081**) so this is a one-liner rather than the
below.

### 1. Export the real script sources

`SurrealDebugger` can dump the `.uc` text buffers embedded in each package:

```
SurrealDebugger <game-root>
> export scripts <PackageName>
```

Output lands in `<game-root>/<PackageName>/Classes/*.uc`. Two cautions, both filed as **BUG-079**:

- **Pass exactly one package per command.** Two or more arguments is an infinite loop.
- **Do not use the no-argument form.** One unreadable package aborts the whole run — it dies on
  `AchtungDieKurve` (UT99) and on `Crashsite2` (Unreal Gold, see BUG-046).

Because the exporter writes into the game root, point it at a scratch directory whose `System/`, `Maps/`,
`Music/`, `Sounds/` and `Textures/` are symlinks to the real install, rather than at the install itself.

Packages that carry script, for reference:

- **UT99 v436** — Core, Engine, UnrealShare, UnrealI, Botpack, UWindow, UMenu, UTMenu, UBrowser, UTBrowser,
  IpDrv, IpServer, UWeb, Fire, relics, UTServerAdmin.
- **Unreal Gold v226** — Core, Engine, UnrealShare, UnrealI, UWindow, UMenu, IpDrv, IpServer, Fire.

### 2. Diff declarations against registrations

Extract every `native(N)` from the exported `.uc` files and every `RegisterVMNativeFunc_*(..., N)` from
`SurrealEngine/`, then compare the index sets.

**Compare by index, not by name.** The engine's C++ handler names do not match the script names for
operators — script `Dot` and `Cross` are registered as `Dot_VectorVector` (219) and `Cross_VectorVector`
(220). A name-based diff reports those as missing and is wrong. The index diff is authoritative.

Two regex traps that cost time the first time round, worth writing down:

- Do not put `\b` after the optional index group in `native(?:\s*\(\s*(\d+)\s*\))?`. After a matched
  `native(233)` the next character is `)` followed by a space — two non-word characters, so `\b` fails and
  every *indexed* native is silently skipped. That failure mode looks exactly like "coverage is complete".
- Operator declarations put the return type between the `operator(N)` keyword and the symbol
  (`native(116) static final operator(24) bool > ( string A, string B )`), so a name-capturing pattern will
  not match them at all. Again: index diff.

The working script used for this run is not checked in — it is thirty lines and BUG-081 supersedes it.
