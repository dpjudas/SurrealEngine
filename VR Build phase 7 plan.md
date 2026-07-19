# VR Build — phase 7 plan: touch to pick up items

Companion to `VR Build checklist.md` (phase 7) and `VR Build research notes.md`. Written 2026-07-19 on `vr_renderer`, after phase 6b landed.

**Goal:** reaching out and touching a floating pickup with a controller collects it, the same way walking the pawn over it does. Engine-side only — no UnrealScript edits.

---

## 1. What already exists (verified this session, do not re-derive)

The whole mechanism phase 7 needs was built in `e7f7b09d` and is sitting in `VR/VRHands.{h,cpp}`:

- `VRHands::Tick()` (`VRHands.cpp:68`) runs from `Engine::Run` (`Engine.cpp:223-224`), *after* `PlayerCalcView`/`OverrideViewAfterCalcView` and before rendering, so hand poses match the frame about to be drawn.
- `VRHands::UpdateTriggerContacts` (`VRHands.cpp:179`) already does exactly the required query: `pawn->XLevel()->Collision.CollidingActors(pose.Position, HandRadius(), HandRadius())`, filters, tracks contacts across frames in `Contacts[hand]` (`GCRoot<UActor>`), and dispatches `CallEvent(actor, EventName::Touch, { ObjectValue(pawn) })` **only on entering**, paired with `UnTouch` on leaving.
- The filter that stops it from being pickup-capable today is one function: `VRHands::IsTriggerLike` (`VRHands.cpp:46`) → `actor->IsA("Triggers")`. Its comment explicitly names the reason it is narrow (a fingertip on a Teleporter would rip the player across the map) — so widening must stay a *whitelist*, never "anything touchable".
- Events are dispatched **as the player pawn**, not as a hand actor. This is forced by script (`Trigger.IsRelevant`, `Mover.BumpButton` both gate on `Pawn(Other).bIsPlayer`) and it is exactly what a pickup needs too: `Pickup.ValidTouch` and `SpawnCopy(Pawn(Other))` want a player pawn.
- `IsBodyTouching` (`VRHands.cpp:58`) already prevents double-firing anything the pawn's own cylinder is inside.
- The overlap path is safe for pickups: `OverlapTester::CollidingActors(origin, height, radius)` (`Collision/TopLevel/OverlapTest.cpp:141`) filters only on the collision hash + `CylinderActorOverlap`; it does **not** require `bBlockActors`. Only brush actors are refused (`CylinderActorOverlap` bails on `actor->Brush()`), which is why movers needed the separate sweep — pickups are mesh actors, so the overlap path is correct for them and no sweep is needed.
- Haptics exist: `VRSubsystem::Haptic(hand, amplitude, duration=0)` (`VRSubsystem.h:160`), used at `VRPlayerInput.cpp:143` and `VRWheel.cpp:44,217` with amplitude `0.5f` for a single runtime-minimum tick.
- Everything in UE1 that can be picked up derives from `Inventory` (`Inventory` → `Pickup` → Health/Ammo/Armor/…, and `Inventory` → `Weapon`), and `UInventory` is already a first-class C++ class (`UObject/UActor.h:856`) with `bHeldItem()`, `bSleepTouch()`, `bTossedOut()`, `bActivatable()` accessors. `UActor::Owner()` is at `UActor.h:612`, `pawn->Inventory()` at `UActor.h:582`.

⇒ **Phase 7 is a filter widening plus a success signal, not a new subsystem.** Expect ~120 lines touched in `VRHands.{h,cpp}` and ~15 lines of settings plumbing.

---

## 2. Step 0 — settle the script contract before writing code

Two questions decide whether step 3 needs an engine-side guard. Both are answered by decompiling, using the reusable recipe in the research notes ("How this was verified"): drive `SurrealDebugger` from a `pty.fork()`, one command every ~3 s, and scope the export or it dies:

```
export scripts Engine
```

Output lands in `<gameRootFolder>/Engine/Classes/*.uc`. Do this for **both** Unreal Gold and UT99 GOTY — phase 6 found the two diverge (`SwitchWeapon`'s tail), so assume nothing transfers.

Read `Pickup.uc` (states `Pickup` and `Sleeping`), `Inventory.uc`, `Weapon.uc` (state `Pickup`) and answer:

1. **Does `ValidTouch` already reject a touch through a wall?** If it traces from `Other.Location` (the *pawn's* location, since we dispatch as the pawn), then the wall case is already handled by script and the hand cannot reach through geometry — the pawn body is the reference, not the hand. If there is no such trace, add the guard in step 3.4.
2. **What does `ValidTouch` gate on** besides being a player — `Health > 0`, `bCanPickupInventory`, `Level.Game.PickupQuery`, `bHidden`/state? Whatever it is, it is the authority; the C++ filter must not try to replicate it, only to avoid dispatching where dispatch is obviously wrong.
3. **Confirm the `Sleeping` state's `bSleepTouch` behaviour** (an item that has been picked up and is waiting to respawn stays collidable but hidden, and a `Touch` while sleeping sets `bSleepTouch` so it is granted on respawn). This drives decision D3 below.

Record the answers in `VR Build research notes.md` under "Verified facts" — this is the third phase in a row that has needed the same decompile.

---

## 3. Implementation

### 3.1 Turn the whitelist into a classification (`VRHands.h`)

Replace `static bool IsTriggerLike(UActor*)` with:

```cpp
// What a hand is allowed to do to an actor it overlaps. Deliberately a whitelist: see the note on
// IsTriggerLike's replacement below - widening this to "anything touchable" hands the player a
// Teleporter they only meant to brush past.
enum class HandTarget { None, Trigger, Pickup };
static HandTarget Classify(UActor* actor);
```

and give `Contact` a `HandTarget Kind` field, so leaving/entering bookkeeping stays one list per hand rather than two parallel ones.

`Classify` body:

```cpp
if (actor->IsA("Triggers")) return HandTarget::Trigger;   // unchanged, and checked first
if (actor->IsA("Inventory") && actor->Owner() == nullptr) return HandTarget::Pickup;
return HandTarget::None;
```

`Owner() == nullptr` is the "lying in the world, not in somebody's pockets" test. Items carried by a pawn have their owner set and their collision off, so this is belt-and-braces — but it is the cheap guard that stops a hand from stripping a weapon out of a monster mid-fight if any game ever leaves a held item collidable. Do **not** additionally filter on `bHidden()`: a hidden-but-collidable pickup is a *sleeping respawner*, and touching it is how `bSleepTouch` gets set — the same thing the body does by standing on the spot (see D3).

### 3.2 Rename and widen the contact pass (`VRHands.cpp`)

`UpdateTriggerContacts` → `UpdateContacts`. The structure is already right; the changes are:

- In the overlap loop (`VRHands.cpp:197-203`), replace `!IsTriggerLike(actor)` with `Classify(actor) == HandTarget::None` and remember the kind alongside the actor in the local `overlapping` array (make it `Array<std::pair<UActor*, HandTarget>>` or a tiny local struct).
- Keep every existing guard verbatim: `actor == pawn`, `bDeleteMe()`, `bCollideActors()`, `IsBodyTouching`.
- Keep the record-then-dispatch split (`VRHands.cpp:226-257`) exactly as it is. It exists so a script reacting to one event cannot resize the array the rest are being read out of, and pickups make that *more* likely, not less — `SpawnCopy` + `PickupFunction` can switch weapons and print messages.
- Keep the `pawn->bDeleteMe()` bail between dispatches (`:250`) and the per-actor `bDeleteMe()` skip (`:254`). A health pickup destroying itself mid-batch is the normal case here, not an exotic one.
- `UnTouch` on leaving: **keep it symmetric for both kinds.** The body sends `UnTouch` when it steps off a pickup; a pickup state that does not implement `UnTouch` simply ignores it. Special-casing buys nothing and costs a divergence from what the body does.

### 3.3 Detect a successful grab and buzz (new, in `UpdateContacts`)

There is no return value from `CallEvent` to read, so success is inferred from observable state either side of the dispatch. Snapshot before, compare after:

```cpp
// Cheap fingerprint of the pawn's inventory chain: head pointer + length, capped so a corrupted
// or cyclic chain can't hang the frame.
static void SnapshotInventory(UPlayerPawn* pawn, UInventory*& head, int& count);
```

Treat the touch as a successful pickup if **any** of:

- the chain fingerprint changed (a weapon or item was added — `SpawnCopy` case), **or**
- `actor->bDeleteMe()` (a health/ammo pickup that consumed itself), **or**
- `actor->bCollideActors()` went false or `bHidden()` went true (it entered a `Sleeping`/respawn state).

The three cover the distinct outcomes: ammo merged into an existing `Ammo` object grows no chain but always disposes of the pickup actor; a weapon adds to the chain; a non-respawning item deletes itself. On success:

```cpp
engine->vr->Haptic(hand, 0.7f); // a grab is a firmer event than a wheel tick's 0.5f
```

Guard the `engine->vr` deref the way `VRWheel` does. Everything else about a failed touch (full health, `PickupQuery` refused it) stays silent — the contact is recorded regardless, so a refused item does not re-fire every frame while the hand rests inside it. **That non-repeat property is the single most important behavioural requirement of this phase**, and it comes for free from the existing contact tracking; do not "simplify" it into a per-frame overlap test.

### 3.4 Wall guard (only if step 0 says script does not do it)

If `ValidTouch` turns out not to trace, add before dispatching a `Pickup` touch:

```cpp
// The hand can be on the far side of a thin wall the body cannot reach through. Trace from the
// pawn, not from the hand: the pawn is who is being credited with the touch, and it is the same
// reference point the script's own reach checks use.
CollisionHitList hits = pawn->XLevel()->Collision.Trace(pawn->Location(), actor->Location(), 0.0f, 0.0f, false, true, false);
```

and skip the dispatch if anything world-geometry (`hit.Actor == nullptr`) is in the way. Triggers keep the current no-trace behaviour — reaching a hand *into* a trigger volume through its own boundary is the point of them.

### 3.5 Gating (in `VRHands::Tick`, or at the top of `UpdateContacts`)

Skip the `Pickup` classification (leave `Trigger` alone) for a hand when:

- a wheel is open on that hand — `engine->vrWheel` is already the owner of that state (`VRWheel` knows its `Hand`); reaching across the wheel disc to a weapon entry must not also grab the ammo box the physical hand happens to be inside.
- the setting in 3.6 disables that hand.

The existing menu/`bNoDrawWorld`/dead-pawn bails at `VRHands.cpp:84-95` already cover the rest and need no change.

### 3.6 Launcher setting — which hand grabs (checklist item 4)

**Recommendation: both hands, on by default, with one knob to narrow it.** Both is what the player expects from every VR game; the failure mode it invites (bumping a pickup with the gun hand mid-fight) is exactly what walking over a pickup already does in the desktop game, so it is not a new class of surprise.

One int, mirroring how `WeaponHand` is already done:

```cpp
// LauncherSettings.h, in the VR struct next to HandColliderRadius (:170)
int PickupHands = 2; // 0 = off, 1 = off-hand only, 2 = both
```

Wire all four places, and check all four — phase 6 shipped a knob that silently did not persist because the JSON half was missed:

1. `LauncherSettings.h` — the field, with the comment above.
2. `LauncherSettings.cpp` load (~`:211`, next to `HandColliderRadius`), clamped to 0..2.
3. `LauncherSettings.cpp` save (~`:372`).
4. `UI/Launcher/VRSettingsPage.cpp` — the control (`:239`-ish for creation/populate, `:429`-ish for read-back).

Read it in `VRHands` through a small helper next to `HandRadius()` (`VRHands.cpp:18`), not by touching `LauncherSettings` at each call site.

### 3.7 Optional, only if in-headset testing asks for it

Do not build these up front:

- **Grabbable highlight** — a wireframe ring on the nearest grabbable within reach, drawn the way `RenderSubsystem` already draws the hand balls (`RenderSubsystem.cpp`, from `e7f7b09d`) and `VRWheel`'s highlight ring (`Draw3DLine`). Only worth it if items turn out to be hard to hit.
- **A reach bonus** — a pickup radius larger than `HandRadius()`. Resist it: the hand is drawn at its collider radius precisely so what you see is what touches, and a bonus breaks that contract. Prefer raising `VR.HandColliderRadius` (default 6) if reach is the problem.

---

## 4. Decisions taken here (and why)

- **D1 — dispatch `Touch`, do not reimplement collection.** Non-negotiable and already the checklist's wording. `SpawnCopy`/`PickupFunction`/`PickupQuery`/pickup messages/respawn scheduling all live in script; any C++ reimplementation would diverge per game and per item.
- **D2 — whitelist `Inventory`, not "touchable".** Same reasoning as the existing `Triggers` narrowing. `Inventory` is the exact class that owns pickup behaviour, and its subclass tree is the full set of things a player is meant to be able to take.
- **D3 — do not filter hidden pickups.** A hidden collidable `Inventory` is a sleeping respawner, and touching it sets `bSleepTouch` so the player is granted it when it comes back — precisely what standing on the spot does. Filtering it out would make the hand *less* faithful than the body. (Revisit if step 0 shows `Sleeping` does something else in Unreal Gold.)
- **D4 — both hands by default** (3.6).
- **D5 — no sweep for pickups.** Movers needed one because they are brushes the overlap test refuses; pickups are mesh actors the overlap test answers for, and an item missed by a fast hand swipe is a non-event — the player swipes again. Adding a sweep would also mean grabbing everything along the arm's path, which is the opposite of precise.

---

## 5. Test plan

Build, then copy `build/SurrealEngine` into `build/Unreal Gold/System/` (and the UT99 equivalent) before launching — the launcher runs from the game folder.

**Interactive VR runs must use a 240 s timeout** (CLAUDE.md): the user needs minutes in-headset and quits manually.

1. **Plumbing, no headset (short timeout, fine).** Confirm the desktop game still starts, still picks items up by walking, and that `VRHands` compiles/links with VR disabled. `Classify`'s trigger branch must be byte-identical in behaviour — phase 3's mover/trigger work is hardware-verified and must not regress.
2. **Unreal Gold, campaign start** (Vortex Rikers → `NyLeve`'s Falls) — the same route phase 6b was tuned on. Checks: the Translator/Dispersion Pistol picked up by hand; the pickup message and HUD reaction fire once, not once per frame; the haptic tick lands on the grab and not on a refusal.
3. **UT99 GOTY, `DM-Deck16][`** — dense ammo/health/armor. Checks: health at full health refuses silently and does **not** re-fire while the hand rests in it; ammo merges and the pickup disappears; a respawning pickup taken by hand comes back on schedule; picking up a weapon by hand switches to it correctly and phase 5's aim/`FireOffset` still tracks the new weapon.
4. **Interaction with phase 6** — open the weapon wheel with a hand parked inside a pickup: nothing is grabbed while the wheel is open, and the grab works again once it closes.
5. **Wall case** — stand next to a pickup with a wall between and push the hand through. Either script refuses it (step 0 outcome 1) or 3.4's guard does.
6. **Regression** — one pass on the phase-3 trigger/mover behaviour: a proximity trigger fired by reaching in, a bump-button pushed by hand.

---

## 6. Known risks

- **Re-entrancy.** Pickup script does more work than trigger script (spawns, weapon switch, HUD messages), so the existing "record first, dispatch second" ordering and the `bDeleteMe()` bails are load-bearing here in a way they were not for triggers. Do not restructure them.
- **The pawn root.** `GCRoot<UPlayerPawn> pawnRoot(pawn)` at `VRHands.cpp:103` is what keeps the pawn alive across dispatches. Pickups can kill a pawn (a booby-trapped item, a scripted event); leave the root in place.
- **Divergence between the two games.** Phase 6 found `SwitchWeapon` differing between Unreal Gold and UT99. Assume `Pickup.uc` may differ too; step 0 exports both.
- **`CollidingActors` cost.** The filter widens from `Triggers` to `Triggers|Inventory`, both cheap `IsA` calls on an already-computed overlap set. No new query per frame. Non-issue, noted so nobody re-measures it.

---

## 7. Out of scope

- The standing `VulkanRenderDevice::~VulkanRenderDevice()` crash on clean exit (phase 6 note, Bugtracker item) — unrelated.
- The Translator-vanishing-across-level-transition investigation (phase 6b note) — unrelated, still owed a dedicated session.
- Throwing/dropping items by hand, and physically holding a picked-up item before it enters the inventory. Both are natural sequels to this phase; neither is needed for "reach out and take it".
- Phase 8 (weapon aim indicator) — independent of this.
