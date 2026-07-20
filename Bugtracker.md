# Bugtracker

Merged defect list for Surreal Engine. Sources folded in:

- the previous free-form `Bugtracker.md` list — tagged **[BT]**
- `Docs/Status.md` "General Engine bugs" and the per-game "Known bugs" sections — tagged **[ST]**
- `Docs/VR.md` "Known limitations" — tagged **[VR]**

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
| [WP-1](#wp-1--save-load-and-level-travel-persistence) | Save/load and level travel persistence | S1 | 7 |
| [WP-2](#wp-2--weapon-fire-semantics) | Weapon fire semantics | S2 | 3 |
| [WP-3](#wp-3--movers-collision-and-physics) | Movers, collision and physics | S2 | 7 |
| [WP-4](#wp-4--vr-presentation-polish) | VR presentation polish | S3 | 4 |
| [WP-5](#wp-5--stability-and-session-state) | Stability and session state | S2 | 5 |
| [WP-6](#wp-6--rendering-fidelity) | Rendering fidelity | S3 | 8 |
| [WP-7](#wp-7--ai-behaviour) | AI behaviour | S3 | 2 |
| [WP-8](#wp-8--parked) | Parked | S5 | — |

---

## WP-1 — Save/load and level travel persistence

**Severity S1.** Everything that makes a saved or travelled-into level come back wrong. These are grouped
together because the investigation points at one shared root cause: `UObject::Save`
(`SurrealEngine/UObject/UObject.cpp:126`) writes a **zeroed** state frame for every object flagged
`HasStack`, so no actor in a loaded level has a script state. Execution plan:
[`WP1-SaveLoad-Plan.md`](WP1-SaveLoad-Plan.md).

**Status 2026-07-20:** plan phases 1, 2 and 3 are complete — BUG-001 … BUG-004, BUG-006 and BUG-007 all
fixed. Phase 3 additionally fixed a latent gap it was asked to check: the `LevelInfo->NextURL` routes in
`Engine::Run` never set `ClientTravelInfo.TravelType`, which is assigned only in `ClientTravel`, so they
inherited stale state and the `bNextItems` branch — whose whole purpose is carrying inventory — silently
discarded it. Each branch now states its intent. (The teleporter route was never affected: `GameInfo.uc`'s
`SendPlayer` calls `ClientTravel(URL, TRAVEL_Relative, true)`, so it always set the type correctly.)

Still open: BUG-005 (phase 4). The UT99 v436 regression pass the plan calls for has not run — everything so
far is verified against Unreal Gold only, and phase 1 touches the load path for *every* package, not just
save files. Phase 3's verification was automated, not played: worth a hands-on confirmation that the
Translator is usable, not merely present, after a real teleporter transition.

| ID | Src | Sev | Defect |
| --- | --- | --- | --- |
| BUG-001 | BT | S1 | ~~Loading a saved game crashes the engine after a few seconds.~~ **FIXED (WP-1 phase 1, 2026-07-19)** — `VisibleMesh::DrawDebugInfo` dereferenced `pawn->StateFrame->LatentState` unguarded, once per pawn-with-mesh per frame. Harmless before phase 1 (every actor always had *some* `StateFrame`); afterwards a pawn with no active state at save time correctly loads with `StateFrame == nullptr` and the first one drawn killed the renderer. Fixed with a null check. |
| BUG-002 | BT | S1 | ~~On a loaded level it is impossible to destroy glass or activate movers.~~ **FIXED (WP-1 phase 1, user-confirmed 2026-07-20)** — both halves. Root cause was the zeroed state frame in `UObject::Save`: with no script state, a mover's `Trigger` dispatched nowhere and breakable glass's damage handler was unreachable. Fixed by real state-frame serialization, plus `GotoState` setting `HasStack` (so states entered during play are actually written) and `RelinkBasedActor` (so movers still know who is riding them). |
| BUG-003 | ST | S1 | ~~Inventory from loaded saves does not transfer to the next map.~~ **FIXED (WP-1 phase 3, 2026-07-20)** — no separate cause of its own; it was BUG-004 plus the phases 1–2 work. Verified automatically: give items → save → load → travel now arrives with every item intact and correctly `Idle2`/hidden. |
| BUG-004 | BT | S1 | ~~The Translator is lost on the first→second level transition (gone from the item list).~~ **FIXED (WP-1 phase 3, 2026-07-20)** — it was never lost in transit; it arrived still in state `Pickup` with `bHidden=0`, i.e. a world pickup rather than an inventory item. `Actor.uc:111` declares `var Inventory Inventory;` with **no `travel` keyword** — UE1 never carries the chain as data, it rebuilds it in `Inventory.TravelPreAccept` → `GiveTo` → `AddInventory`. `GetAllTravelProperties` force-includes `Inventory` so `Create` can walk the chain, and `Accept` was then also writing those pointers back, pre-linking the chain before `TravelPreAccept` ran. `Translator.TravelPreAccept` skips its `Super` call when `FindInventoryType(class) != None` — with the chain already wired it found *itself*, so it never got `BecomeItem()`/`GotoState('Idle2')`. Fixed by not restoring `Inventory` in `Accept` unless it is genuinely `Travel`-flagged (it is, in Deus Ex — that path is unchanged). |
| BUG-005 | ST | S2 | Saving packages (`.u*`, game saves) is not fully implemented. |
| BUG-006 | — | S2 | ~~`Engine::GameInfo` is only assigned in `LoadMap`, so it dangles at the previous level after `LoadFromSaveFile`.~~ **FIXED (WP-1 phase 2, 2026-07-19)** — `LoadFromSaveFile` re-points `GameInfo` at `LevelInfo->Game()` after `GetLevelObject()` (`Engine.cpp:792`), and throws if the save carries no GameInfo actor rather than limping on. |
| BUG-007 | — | S1 | ~~Saving intermittently crashes the engine.~~ **FIXED (WP-1, 2026-07-20)** — `UObject::Save` dereferenced `StateFrame->Func` unguarded. A `StateFrame` outlives its state: `GotoState("")` keeps the frame but nulls `Func`, while `LatentState` is left at its `Continue` default, never `Stop` — so a dormant actor passed the `StateFrame && LatentState != Stop` guard and crashed on `Func->Code`. Latent since the `HasStack` fix started routing dormant actors into that block; `bTriggerOnceOnly` movers are the common producer. Fixed by also checking `Func` (and `Func->Code`, null for a bytecode-less state), which writes a null `func` and correctly restores the actor as dormant. |

## WP-2 — Weapon fire semantics

**Severity S2.** How a held trigger turns into a shot.

| ID | Src | Sev | Defect |
| --- | --- | --- | --- |
| BUG-010 | BT, VR | S2 | Charging weapons (Dispersion Pistol, Impact Hammer, Rocket Launcher) mishandle the held trigger: the Rocket Launcher fires one rocket immediately and only then starts charging. Same on alt-fire. |
| BUG-011 | VR | S3 | Arcing projectiles (Bio Rifle, grenades) read as mis-aimed at range — the aim indicator draws a straight line while the projectile adds Z velocity and falls. |
| BUG-012 | VR | S3 | Firing with the hand against a wall can spawn the projectile clipped, because `FireOffset` puts the shot origin at the hand. |

## WP-3 — Movers, collision and physics

**Severity S2.** World interaction and the player's body. Related to WP-1's mover symptom but distinct: these
are wrong on a *freshly loaded* map too.

| ID | Src | Sev | Defect |
| --- | --- | --- | --- |
| BUG-020 | ST | S2 | Player-to-decoration and player-to-pawn collision is not properly implemented: the player gets stuck, and on breaking free dies from phantom fall damage. |
| BUG-021 | ST | S2 | Semisolid brushes are finicky — usually the player falls through as if they weren't there. |
| BUG-022 | ST | S2 | It is possible to get stuck on some movers when approached from certain angles. |
| BUG-023 | ST | S2 | Projectiles pass through some movers. |
| BUG-024 | ST | S3 | Some mover buttons are too easy to push (e.g. the ceiling button in the Kevlar Suit room in Vortex Rikers triggers by walking under it). |
| BUG-025 | ST | S3 | Underwater collisions are buggy, especially in Klingon Honor Guard. |
| BUG-026 | ST | S3 | The player can slide through some catwalks in the lava room of DM-Conveyor. |

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
