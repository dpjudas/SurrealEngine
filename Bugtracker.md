# Bugtracker

Merged defect list for Surreal Engine. Sources folded in:

- the previous free-form `Bugtracker.md` list — tagged **[BT]**
- `Docs/Status.md` "General Engine bugs" and the per-game "Known bugs" sections — tagged **[ST]**
- `Docs/VR.md` "Known limitations" — tagged **[VR]**
- the structural review in `Refactor-Plan.md` — tagged **[RF]**

Those documents remain the descriptive references; this file is the ordered work list. Fixed entries are
kept struck through with the fixing phase noted, so the history isn't lost.

## Severity scale

| | Meaning |
| --- | --- |
| **S1 Blocker** | Crashes, loses progress, or makes a supported game unfinishable. |
| **S2 Major** | Core gameplay behaves wrongly; the game is playable but visibly broken. |
| **S3 Moderate** | Noticeable wrongness that does not block play. |
| **S4 Minor** | Cosmetic or convenience. |
| **S5 Parked** | Out of scope for the playable targets (UT99 v436, Unreal Gold v226), or a whole missing feature rather than a defect. |

## Work packages, in execution order

| WP | Theme | Severity | Items |
| --- | --- | --- | --- |
| [WP-1](#wp-1--save-load-and-level-travel-persistence) | Save/load and level travel persistence | S1 | 9 |
| [WP-2](#wp-2--weapon-fire-semantics) | Weapon fire semantics | S2 | 3 |
| [WP-3](#wp-3--movers-collision-and-physics) | Movers, collision and physics | S2 | 7 |
| [WP-4](#wp-4--vr-presentation-polish) | VR presentation polish | S3 | 4 |
| [WP-5](#wp-5--stability-and-session-state) | Stability and session state | S2 | 5 |
| [WP-6](#wp-6--rendering-fidelity) | Rendering fidelity | S3 | 8 |
| [WP-7](#wp-7--ai-behaviour) | AI behaviour | S3 | 2 |
| [WP-9](#wp-9--unrealscript-vm-correctness-and-robustness) | UnrealScript VM correctness and robustness | S2 | 8 |
| [WP-10](#wp-10--tooling-and-diagnostics) | Tooling and diagnostics | S4 | 1 |
| [WP-8](#wp-8--parked) | Parked | S5 | — |

---

## WP-1 — Save/load and level travel persistence

**Severity S1.** Everything that makes a saved or travelled-into level come back wrong. These were grouped
together because they shared one root cause: `UObject::Save` wrote a **zeroed** state frame for every object
flagged `HasStack`, so no actor in a loaded level had a script state. That, and everything it masked, is now
fixed; the per-bug entries below carry the root cause and fix for each. (The separate `WP1-SaveLoad-Plan.md`
and `WP1-Handover-2026-07-19.md` working documents were retired on 2026-07-20 once phases 1–3 landed —
everything still live from them is folded into this section and into BUG-005's scope note.)

**Status 2026-07-20:** phases 1, 2 and 3 are complete — BUG-001 … BUG-004, BUG-006, BUG-007, BUG-008 and
BUG-009 all fixed, and **level transfer is user-confirmed working in real gameplay**. Phase 3 additionally
fixed a latent gap: the `LevelInfo->NextURL` routes in
`Engine::Run` never set `ClientTravelInfo.TravelType`, which is assigned only in `ClientTravel`, so they
inherited stale state and the `bNextItems` branch — whose whole purpose is carrying inventory — silently
discarded it. Each branch now states its intent. (The teleporter route was never affected: `GameInfo.uc`'s
`SendPlayer` calls `ClientTravel(URL, TRAVEL_Relative, true)`, so it always set the type correctly.)

**UT99 v436 regression pass run 2026-07-20.** It found BUG-008, a hard crash on *every* save load in UT99
that Unreal Gold structurally could not expose (package version 61 vs 68) — the exact reason the plan
required a second game. Fixed; save → load → travel now completes on UT99 across all three travel routes
(direct `ClientTravel`, save+load+travel, and `NextURL`/`bNextItems`), and Unreal Gold is unchanged. The pass
also turned up BUG-009, an inconsistent travel-map key, since fixed.

Still open: **BUG-005 only** — see its scope note in the table below.

Note for anyone testing travel on UT99: **UT does not carry inventory between maps by design**, so an empty
inventory after a UT level change is correct and is not evidence of a travel bug. Use Unreal Gold to test
that inventory actually transfers.

| ID | Src | Sev | Defect |
| --- | --- | --- | --- |
| BUG-001 | BT | S1 | ~~Loading a saved game crashes the engine after a few seconds.~~ **FIXED (WP-1 phase 1, 2026-07-19)** — `VisibleMesh::DrawDebugInfo` dereferenced `pawn->StateFrame->LatentState` unguarded, once per pawn-with-mesh per frame. Harmless before phase 1 (every actor always had *some* `StateFrame`); afterwards a pawn with no active state at save time correctly loads with `StateFrame == nullptr` and the first one drawn killed the renderer. Fixed with a null check. |
| BUG-002 | BT | S1 | ~~On a loaded level it is impossible to destroy glass or activate movers.~~ **FIXED (WP-1 phase 1, user-confirmed 2026-07-20)** — both halves. Root cause was the zeroed state frame in `UObject::Save`: with no script state, a mover's `Trigger` dispatched nowhere and breakable glass's damage handler was unreachable. Fixed by real state-frame serialization, plus `GotoState` setting `HasStack` (so states entered during play are actually written) and `RelinkBasedActor` (so movers still know who is riding them). |
| BUG-003 | ST | S1 | ~~Inventory from loaded saves does not transfer to the next map.~~ **FIXED (WP-1 phase 3, 2026-07-20)** — no separate cause of its own; it was BUG-004 plus the phases 1–2 work. Verified automatically (give items → save → load → travel arrives with every item intact and correctly `Idle2`/hidden) and **user-confirmed in real gameplay 2026-07-20**. |
| BUG-004 | BT | S1 | ~~The Translator is lost on the first→second level transition (gone from the item list).~~ **FIXED (WP-1 phase 3, 2026-07-20)** — it was never lost in transit; it arrived still in state `Pickup` with `bHidden=0`, i.e. a world pickup rather than an inventory item. `Actor.uc:111` declares `var Inventory Inventory;` with **no `travel` keyword** — UE1 never carries the chain as data, it rebuilds it in `Inventory.TravelPreAccept` → `GiveTo` → `AddInventory`. `GetAllTravelProperties` force-includes `Inventory` so `Create` can walk the chain, and `Accept` was then also writing those pointers back, pre-linking the chain before `TravelPreAccept` ran. `Translator.TravelPreAccept` skips its `Super` call when `FindInventoryType(class) != None` — with the chain already wired it found *itself*, so it never got `BecomeItem()`/`GotoState('Idle2')`. Fixed by not restoring `Inventory` in `Accept` unless it is genuinely `Travel`-flagged (it is, in Deus Ex — that path is unchanged). **User-confirmed in real gameplay 2026-07-20.** |
| BUG-005 | ST | S2 | Saving packages (`.u*`, game saves) is not fully implemented. **Scope** (from the now-retired plan): `PackageWriter` (`Package/PackageWriter.cpp`) writes header, objects, and the name/export/import tables. Known gaps visible in the source — `WriteHeader` writes `GenerationCount = 0` and no generations, and `Save` renames the previous file to `.old` with the failure swallowed. Enumerate what a v436 / v226 save actually needs versus what is written, and fix only what the two target games read back; do **not** chase general-purpose `.u` authoring, which is a separate goal. Note BUG-008 was exactly this class of defect (a writer/reader mismatch) found by accident — a systematic `Read*`/`Write*` op-order diff of each `Load`/`Save` pair is a cheap way to find the rest. |
| BUG-006 | — | S2 | ~~`Engine::GameInfo` is only assigned in `LoadMap`, so it dangles at the previous level after `LoadFromSaveFile`.~~ **FIXED (WP-1 phase 2, 2026-07-19)** — `LoadFromSaveFile` re-points `GameInfo` at `LevelInfo->Game()` after `GetLevelObject()` (`Engine.cpp:792`), and throws if the save carries no GameInfo actor rather than limping on. |
| BUG-008 | — | S1 | ~~Loading any save crashes on UT99 (and any package version > 61) with "Could not cast object Class (class Class) to UActor".~~ **FIXED (WP-1 regression pass, 2026-07-20)** — `UModel::Save` wrote the zone count with `WriteIndex` (a variable-length compact index) while `UModel::Load` reads it with `ReadInt32`. 64 zones encode as 2 bytes, not 4, so the whole rest of the model stream was misaligned and the first `ZoneActor` resolved to a garbage export. Masked entirely on Unreal Gold: its maps are package version **61** and take the `<= 61` OldFormat branch, which has no zone count at all — UT99 maps are version **68**. Found only because the regression pass ran a second game. |
| BUG-009 | — | S3 | ~~The travel map is keyed inconsistently: `CreateTravelInfo` keyed it by `pawn->PlayerReplicationInfo()->PlayerName()` (**"Player1"** on UT99) while `LoginPlayer` looks it up by the destination URL's `Name` option (**"Player"**), so the lookup missed and logged "Skipping travel transfer. Player 'Player' not found in travel info".~~ **FIXED (2026-07-20)** — both sides now derive the key through one `Engine::GetTravelPlayerName`, so they agree by construction. The URL's `Name` is authoritative because that is what `GameInfo.Login` names the arriving pawn; the departing pawn's replication-info name is not, since game script is free to change it. Unreal Gold only ever worked by coincidence, the two names happening to agree there. **Severity is S3, not S2: this was never user-visible on UT99**, because UT deliberately does not carry inventory between maps — the engine was skipping a transfer whose result the game discards anyway. It is a latent correctness bug that would bite any UT-based mod or game that does travel with items. |
| BUG-007 | — | S1 | ~~Saving intermittently crashes the engine.~~ **FIXED (WP-1, 2026-07-20)** — `UObject::Save` dereferenced `StateFrame->Func` unguarded. A `StateFrame` outlives its state: `GotoState("")` keeps the frame but nulls `Func`, while `LatentState` is left at its `Continue` default, never `Stop` — so a dormant actor passed the `StateFrame && LatentState != Stop` guard and crashed on `Func->Code`. Latent since the `HasStack` fix started routing dormant actors into that block; `bTriggerOnceOnly` movers are the common producer. Fixed by also checking `Func` (and `Func->Code`, null for a bytecode-less state), which writes a null `func` and correctly restores the actor as dormant. |

## WP-2 — Weapon fire semantics

**Severity S2.** How a held trigger turns into a shot.

| ID | Src | Sev | Defect |
| --- | --- | --- | --- |
| BUG-010 | BT, VR | S2 | Charging weapons (Dispersion Pistol, Impact Hammer, Rocket Launcher) mishandle the held trigger: the Rocket Launcher fires one rocket immediately and only then starts charging. Same on alt-fire. |
| BUG-012 | VR | S3 | Firing with the hand against a wall can spawn the projectile clipped, because `FireOffset` puts the shot origin at the hand. |

## WP-3 — Movers, collision and physics

**Severity S2.** World interaction and the player's body. Related to WP-1's mover symptom but distinct: these
are wrong on a *freshly loaded* map too.

**Status 2026-07-20: closed.** BUG-020, BUG-022, BUG-023, BUG-025 and BUG-062 are fixed and confirmed in
real gameplay; BUG-021 and BUG-024 remain open with their findings recorded below. (The `WP3-Collision-Plan.md`
working document was retired in this commit — everything still live from it is folded into the entries below.)

Two lessons worth carrying forward:

- **Measure before fixing.** The package's highest-confidence hypothesis — that extent sweeps cannot hit
  semisolid geometry — was wrong, and a probe said so before any code was written. The one that finally
  mattered was found the same way: instrumenting a real play session, not reading code. Three sessions found
  nothing because the detector looked for a dead stop while the actual symptom was a *slowdown*; it only
  worked once it measured what the player actually reported.
- **Symmetric block tests are a trap.** Two separate defects (BUG-023, and the mover-slide dead end under
  BUG-022) came from asking whether *both* sides of a collision consent to blocking. Level geometry does not
  work that way, and the bugs stayed hidden because `PlayerPawn` happens to set every flag involved.

| ID | Src | Sev | Defect |
| --- | --- | --- | --- |
| BUG-020 | ST | S2 | ~~Player-to-decoration and player-to-pawn collision is not properly implemented: the player gets stuck, and on breaking free dies from phantom fall damage.~~ **FIXED (WP-3 phase 3, 2026-07-20)** — walking physics had an empty `// TODO` where a blocking actor should be handled, so the pawn jammed and re-collided every tick; it now slides along the actor the way it slides along a wall. Cylinder depenetration is decided in `TryMove` rather than in the trace, so the hit still reaches the `Touch` loop — deciding it in the trace silently stopped pickups and proximity triggers from firing. **User-confirmed 2026-07-20.** |
| BUG-021 | ST | S2 | Semisolid brushes are finicky — usually the player falls through as if they weren't there. **OPEN, no repro.** The leading hypothesis (extent sweeps cannot hit semisolid geometry) was **refuted by measurement**: on Vortex2 1222 of 1546 probes stop on the semisolid surface and 6 pass through; on NyLeve 1898 of 2190. Nobody has reproduced this first-hand — it came from `Docs/Status.md`. Do not act on it without a repro. |
| BUG-022 | ST | S2 | ~~It is possible to get stuck on some movers when approached from certain angles.~~ **FIXED (WP-3 phases 1 and 4, 2026-07-20)** — three separate causes: `EncroachingActors` keyed its visited-marker on the mover instead of the candidate, so a mover only ever saw one encroaching actor; it derived its bucket search from the mover's collision cylinder while movers are filed by brush bounding box, so a wide door missed actors near its ends; and walking physics could not slide along a mover at all, because a brush has no collision cylinder and so matched none of the actor-hit cases. **User-confirmed 2026-07-20.** |
| BUG-023 | ST | S2 | ~~Projectiles pass through some movers.~~ **FIXED (WP-3 phase 4, 2026-07-20)** — blocking against a brush required the block flag on *both* sides, and `Engine.Projectile`'s class defaults are `bBlockActors=0`/`bBlockPlayers=0` (read from the loaded CDO), so no mover could ever stop any projectile. Players were unaffected only because `PlayerPawn` sets both. A brush is level geometry, so its own flags now decide. `TickProjectile`'s `HitWall` guard was also `!hit.Actor`, which would have left a stopped rocket hanging in mid-air without detonating. **User-confirmed 2026-07-20.** |
| BUG-024 | ST | S3 | Some mover buttons are too easy to push (e.g. the ceiling button in the Kevlar Suit room in Vortex Rikers triggers by walking under it). **OPEN — user-confirmed 2026-07-20** (Vortex Rikers, the first level of Unreal). **Leading hypothesis, not yet acted on:** `TickWalking` does the classic step-up/move/step-down (`UActor.cpp:706`), and the forward move runs while the pawn is raised by `MaxStepHeight`. `TryMove` dispatches `Touch` from every one of those calls, so a pawn effectively touches anything up to a step-height above its real head on every step it takes — which is exactly "triggers by walking under it". Two things ruled out first: the cylinder-vs-cylinder overlap test bounds height correctly (`\|dz\| < hA+hB`), and the trace bounds the cylinder end caps correctly; neither is over-generous. Any fix here changes `Touch` semantics globally, which is the same class of change that regressed pickups in WP-3 phase 3 — get a before/after repro in the Kevlar Suit room rather than fixing blind. |
| BUG-025 | ST | S3 | ~~Underwater collisions are buggy, especially in Klingon Honor Guard.~~ **FIXED (WP-3 phase 1, 2026-07-20)** — the swimming and flying ticks tested `isMoving` with `&&` across the velocity components, so a pawn moving on one axis only (straight up, straight down) counted as stationary and never moved. Also removed a copy-pasted `Velocity().z = 0` that prevented flying pawns accumulating vertical speed. **User-confirmed 2026-07-20** (swimming up and down). |
| BUG-026 | ST | S3 | The player can slide through some catwalks in the lava room of DM-Conveyor. **PARTLY ADDRESSED** — a distinct defect in this exact area was found and fixed (BUG-062). Sliding *through* a catwalk has not been reproduced since; re-test before acting further. |
| BUG-062 | — | S2 | ~~Standing on another actor's collision cylinder makes the player crawl: physics flips between walking and falling every frame.~~ **FIXED (WP-3, 2026-07-20)** — cylinder hit normals came from `normalize(hitpos - actor->Location())`, which is the true surface normal only on a cylinder's curved side; on the flat end caps it tilts outwards by however far the contact sits from the axis. A `BlockAll` actor over a DM-Conveyor walkway reported z=0.685 against the 0.7071 walkable threshold, so the step-down probe called the floor too steep every frame — 32 physics-mode changes in 32 frames, airborne half the time, hence "not stopped, just incredibly slow". Now decided by least penetration, which is also correct for a sweep that begins in contact (fraction 0, where the "hit point" is merely where it started). **User-confirmed 2026-07-20.** |

## WP-4 — VR presentation polish

**Severity S3.** VR-specific presentation and comfort; all cosmetic or convenience, none block play.

| ID | Src | Sev | Defect |
| --- | --- | --- | --- |
| BUG-030 | BT | S3 | The translator model is fully black in the item wheel and in hand when selected (the flare model renders correctly). |
| BUG-031 | VR | S3 | Swimming and flying orient movement by `ViewRotation`, so the player swims toward the aimed hand instead of the body. |
| BUG-032 | BT | S4 | Controller models are far too large — should be ~20% of current size, and a circle with a line is enough (no sphere). |
| BUG-033 | VR | S4 | The HUD tablet is always on the forearm; it cannot be dismissed or hidden. |

## WP-5 — Stability and session state

**Severity S2.** Crashes and state that leaks across a session, outside the save/load path.

| ID | Src | Sev | Defect |
| --- | --- | --- | --- |
| BUG-040 | ST | S1 | Opening a map sometimes crashes with "Failed to spawn the player actor". |
| BUG-041 | ST | S2 | The `viewclass` console command crashes with a null deref. |
| BUG-042 | ST | S3 | Screen-tinting power-ups (Invisibility, Energy Amplifier) leave the tint applied — and accumulate it on re-pickup — until the map changes. |
| BUG-043 | ST | S3 | Some sounds are far too loud (Pulse Rifle secondary, minigun firing). |
| BUG-045 | RF | S2 | **Every `UObject` of every level leaks for the process lifetime — the garbage collector is dead three separate ways and nothing calls it.** (a) `GC::Collect()` has no call sites anywhere in the tree; only `GC::GetStats()` is called, from `RenderCanvas.cpp:653`, so the debug overlay reports a heap that is never swept. (b) `GC::Mark` (`GC/GC.cpp:87-95`) discards the marklist each object returns — `allocation->object()->Mark(marklistout);` ignores the result, and `GC::MarkObject` is a pure function that cannot extend the caller's list in place — so `GC::Mark` always returns null and `Collect`'s `while (marklist)` loop ends after one pass. (c) `UObject::Mark` (`UObject/UObject.cpp:565-570`) is a stub with its whole body commented out. On top of that `PackageManager::UnloadPackage` (`Package/PackageManager.cpp:187-202`) only evicts the package from the open-stream cache and frees no objects, so `Engine::UnloadMap` releases nothing. **Measure before fixing** — instrument `GC::GetStats()` at every `LoadMap`/`UnloadMap` boundary and travel a four-or-five level Unreal Gold chain first; that number is what any fix is judged against. The blocker for making the sweep work is `PropertyDataBlock::Reset` (`UObject.cpp:574-585`), also commented out with `// To do: this crashes as the class might have been destroyed first`. Full options analysis in `Refactor-Plan.md` RP-1. |
| BUG-046 | RF | S2 | Unreal Gold's Return to Na Pali map `Maps/UPak/Crashsite2.unr` fails to load with `ObjectStream::ReadString: Invalid size in Crashsite2`. Found incidentally while exporting all packages, so other UPak maps may be affected too — nobody has swept them. **Leading hypothesis, not verified:** `ObjectStream::ReadString` (`Package/ObjectStream.h:86-97`) throws on any negative length, but `ReadIndex` decodes a sign bit and UE's string serializer uses a **negative count to mean the string is UTF-16**. If that is what this is, the fix is to read `-len` UTF-16 code units rather than throw. Hexdump the offending export and confirm before changing the reader — this is a hypothesis from reading the decoder, not a measurement. |
| BUG-044 | — | S3 | `VulkanRenderDevice::~VulkanRenderDevice` segfaults on every clean shutdown (seen on AMD RADV), after the game has otherwise exited normally. Cosmetic in effect — nothing is lost — but it drops a core on every quit, which masks real crashes and poisons bisects. Pre-existing, predates the WP-1 work. |

## WP-6 — Rendering fidelity

**Severity S3.** Things that draw wrongly.

| ID | Src | Sev | Defect |
| --- | --- | --- | --- |
| BUG-050 | ST | S2 | Third-person weapon meshes are never rendered. |
| BUG-051 | ST | S2 | No dynamic lighting: Dispersion Pistol projectiles and Flares don't illuminate their surroundings. |
| BUG-052 | ST | S3 | Portals mostly work but push players/projectiles in unexpected directions. |
| BUG-053 | ST | S3 | Nali Fruit Seeds and ASMDs placed in a map don't render, though they can be picked up. |
| BUG-054 | ST | S3 | Shock Rifle beams render glitchy (UT). |
| BUG-055 | ST | S3 | Waving water textures at the ends of waterfalls render broken (NyLeve's Falls, DM-ArcaneTemple). |
| BUG-056 | ST | S4 | Mirrors/reflections are buggy, especially at the edges of world geometry. |
| BUG-057 | ST | S4 | ASMD tertiary fire rings render wrong (Unreal Gold). |
| BUG-058 | BT | — | ~~Explosion/impact sprites face the wrong way instead of the camera.~~ **FIXED (VR phase 8)** — sprites billboarded off `ViewRotation` alone, which in VR is only the body-anchor yaw from the phase-5 aim/view split, never the headset pose. New `VisibleFrame::HeadLocalToWorld()` combines the two; applied to `VisibleSprite::Draw` and the wheel's icon fallback. |

## WP-7 — AI behaviour

**Severity S3.** Blocked on unimplemented native functions; large and self-contained.

| ID | Src | Sev | Defect |
| --- | --- | --- | --- |
| BUG-060 | ST | S3 | Bot and ScriptedPawn AI is largely non-functional — the related natives aren't implemented. Pawns only sometimes retaliate and pick up nearby items. |
| BUG-061 | ST | S4 | Bots rotate their whole body (feet off the ground) to look up and down. |

## WP-9 — UnrealScript VM correctness and robustness

**Severity S2.** From the structural review in `Refactor-Plan.md` (RP-2 and RP-3). Two halves: places where
the VM computes a *wrong answer* that script can see, and places where a script error becomes undefined
behaviour, a hang, or silence instead of a diagnosable failure.

**How the conformance half was found.** Both games' real UnrealScript sources were exported with
`SurrealDebugger`'s `export scripts`, and every `native(N)` declaration was diffed against every
`RegisterVMNativeFunc_*` in the engine — 230 indexed natives on UT99 v436, 145 on Unreal Gold v226. Exactly
one index is unregistered on each (BUG-070), which is a much better result than `Docs/Status.md` implies:
the real gap is the ~38 handlers that register successfully and then call `LogUnimplemented` (StatLog,
WebRequest/WebResponse, cache entries, skeletal anim), not missing registrations. **That reframes BUG-060** —
the AI work is stubs to fill in, not natives to wire up. Making this diff a `natives check` commandlet is
tracked as BUG-081.

None of the robustness entries has a known repro; they are latent, and they are listed because they sit
directly under code that the VM work will touch. Fix BUG-074 first — it is what makes the others
straightforward.

| ID | Src | Sev | Defect |
| --- | --- | --- | --- |
| BUG-070 | RF | S2 | String `>` is unregistered on both target games. `Native/NObject.cpp:98` registers `Greater_StrStr` at native index **1186** (a Deus Ex / 227-range index, evidently copy-pasted from that table) while both games declare it at **116** — `Core/Classes/Object.uc:243`, identical in UT99 and Gold. Its four siblings are all correct (`Less_StrStr` 115, `LessEqual_StrStr` 120, `GreaterEqual_StrStr` 121, `EqualEqual_StrStr` 122), and the handler itself (`NObject.cpp:929-932`) is fine and is already registered correctly at 198 under the old-version branch (`NObject.cpp:286`). Any script evaluating `StrA > StrB` hits `Frame::CallNative`'s "Unknown native function" throw. Latent, but this is the **only** unregistered native index in either game and the fix is one line: register 116, keep 1186 for Deus Ex. |
| BUG-072 | RF | S3 | `Level.Year` is off by 1900 and `Level.Month` is 0-based. `Engine.cpp:167-174` assigns `tm_year` and `tm_mon` straight through, but UE1's `LevelInfo.Year` is the calendar year and `Month` is 1-12. Consequences in real script: `UnrealShare/Classes/UnrealSaveMenu.uc:27` does `MonthNames[Level.Month - 1]`, i.e. **`MonthNames[-1]` in January**, in Unreal Gold's save-game menu; `Botpack/Classes/TournamentGameInfo.uc:366-376` and `Engine/Classes/StatLog.uc:96` render dates as e.g. `6/21/126`. `Millisecond` is also hardcoded to 0 (`std::chrono` has what `tm` lacks). While here, `std::localtime` returns a shared static and is not thread-safe — use `localtime_r`/`localtime_s`. |
| BUG-074 | RF | S2 | `Frame::ThrowException` returns to its caller when a debugger is attached, and every call site is written as if it does not. `Frame.cpp:131-139` sets `ExceptionText` and calls `Break()`, which only converts that into a real `Exception::Throw` when `RunDebugger` is unset (`:86-106`); with `SurrealDebugger` attached the break runs and control falls through. Sharpest case is `Frame::Run` (`Frame.cpp:478-479`), where the out-of-range statement index it just diagnosed is then used to index `Statements` anyway — an out-of-bounds read on exactly the condition being reported. Only reachable under the debugger. **Fix this one first:** making `ThrowException` `[[noreturn]]` (breaking before the unwind, not instead of it) is what makes BUG-073 and BUG-075 simple. |
| BUG-073 | RF | S2 | The VM's runaway-instruction guard neither aborts the frame nor exits the loop. `Frame.cpp:474-491` checks `instructionsRetired >= maxInstructions` *inside* the loop and then only logs and calls `Break()` — so a runaway script logs the same line every iteration forever and keeps executing, producing an unbounded log and a hang instead of the intended abort. Without a debugger `Break()` is additionally a no-op here, because `ExceptionText` is empty. UE1's own behaviour is a "runaway loop" script error; that is the model. |
| BUG-075 | RF | S2 | Unbounded script recursion overflows the C++ stack and kills the process with no diagnostic. `Frame::Callstack` (`Frame.h:69`, pushed by `ActiveCallStackFrame` at `:109-113`) has no depth limit, and `ExpressionEvaluator::Eval` recurses on the C++ stack for every nested expression. The bytecode *parser* already guards itself at depth 64 (`Bytecode.cpp:17-18`); the evaluator has no equivalent. UE1 used a 250-frame limit and raised a script error carrying the callstack. |
| BUG-076 | RF | S3 | `string(vector)` and `string(rotator)` do not round-trip. `StringToVector`/`StringToRotator` (`VM/ExpressionEvaluator.cpp:516-544`) parse a bare `"1,2,3"`, but UE1 formats vectors as `X=1.0,Y=2.0,Z=3.0` — so `atof("X=1.0")` returns 0 and every component comes out zero. |
| BUG-077 | RF | S4 | Three conversion operators disagree with UE1, all in `VM/ExpressionEvaluator.cpp`: `BoolToString` (`:571-574`) yields `"1"`/`"0"` via `std::to_string(bool)` where UnrealScript's `string(bool)` is `"True"`/`"False"`; `ObjectToString` (`:581-585`) yields `Package.Name` where UE1 gives the object name alone; `RotatorToString` (`:598-602`) masks each component to 16 bits, so negative angles print as large positives. Check `UProperty::PrintValue` before changing the bool case — the property-export path used by `ConsoleCommand("get ...")` may already be correct, and only the explicit cast wrong. Also `IntToByte`/`FloatToByte` (`:436-439`, `:466-469`) narrow to `uint8_t` with no clamp, so a negative float is UB rather than UE1's wrap. |
| BUG-078 | RF | S3 | `DynamicCast` compares class **names**, not class identity: `ExpressionEvaluator.cpp:358-364` tests `value->IsA(expr->Class->Name)`, so two same-named classes in different packages cast successfully into each other. `MetaCast` (`:210-226`) walks the pointer chain correctly and is the model to follow. |

Four evaluator cases throw unconditionally and their reachability on the two target games is **unverified** —
worth a disassembly sweep before either implementing or documenting them: `LabelTableExpression`
(`:104-108`), `NativeParmExpression` (`:333-336`), `ConstructExpression` (`:682-685`),
`Unknown0x46Expression` (`:481-484`). `LabelTableExpression` is the one that matters: label tables are the
last statement of every state's bytecode (`Bytecode::FindLabelIndex`, `Bytecode.h:31-43`, relies on exactly
that), so a state body that falls off its end without a `Stop` would execute the table and throw.
`Unknown0x15Expression` (`:228-233`) silently returns `Stop` on a Klingon-Honor-Guard guess baked into the
hot path, and deserves the same treatment.

## WP-10 — Tooling and diagnostics

**Severity S4.** The debugger and its commandlets are how the other work packages get evidence, so defects
here cost more than their severity suggests.

| ID | Src | Sev | Defect |
| --- | --- | --- | --- |
| BUG-079 | RF | S4 | `export scripts` is unusable in its two most useful forms. With **two or more** package arguments it loops forever: `ExportCommandlet.cpp:83-91` computes `sep` once and never updates it, so `while (sep != std::string::npos)` spins re-pushing the first name (a single package works only by accident, `sep` being `npos` immediately). With **no** arguments, one unreadable package aborts the entire run — `Could not find package AchtungDieKurve` on UT99, and `ObjectStream::ReadString: Invalid size in Crashsite2` on Unreal Gold (BUG-046), each of which killed a full-tree export that had already succeeded for dozens of packages. Per-package error isolation would let the run finish and *report* the failures, which is the whole point of a diagnostic tool. A missing `Maps/` directory likewise throws out of a `directory_iterator` instead of meaning "no maps". |
| BUG-081 | RF | S4 | There is no way to re-run the native-coverage check that found BUG-070 without hand-scripting an export and a diff. `SurrealDebugger` already loads every package and already holds every registered handler, so a `natives check` commandlet could print the unregistered and `LogUnimplemented`-stubbed sets for the loaded game directly. That turns a conformance audit into a one-line regression check, and it is what would keep a future index typo from surviving as long as BUG-070 did. |

## WP-8 — Parked

**Severity S5.** Not targeted while UT99 v436 and Unreal Gold v226 are the playable goals. Kept here so the
list above stays about defects in supported configurations. Full detail stays in `Docs/Status.md`.

- **Other engine versions** — Unreal Gold 227* and UT 469* have many unimplemented natives/features; 227k_14
  crashes immediately; 227i Translator Scale option does nothing; some UPak natives are missing.
- **Other games** — Deus Ex (partially playable; conversations, DataCubes/books, Load Game menu, HUD text),
  Tactical-Ops (crashes on startup, menus and maps), Klingon Honor Guard (keybinds, botmatch crash), and
  every other detected UE1 game (crash on startup).
- **Missing subsystems** — no networking, no OpenGL renderer, no native-mod support (by design), VM arrays
  and network conditional execution unimplemented.
- **[Linux/ZWidget] Wayland backend** — menu positioning, persistent menus, and no client-side decorations
  on GNOME. Tracked upstream in `SurrealWidgets`.
