# VR Build — phase 6 plan: weapon wheel / item wheel

Companion to `VR Build checklist.md` (phase 6) and `VR Build research notes.md`. This is the detailed
implementation design. Same ground rules as every phase: **engine-side C++ only, the games' UnrealScript is
off limits.** Everything below is reachable from code paths that already exist on the `vr_renderer` branch.

Session date: 2026-07-19. Branch: `vr_renderer`. The Index + VR server are available for testing this phase,
so unlike phases 1–5 the *feel* items can be closed in-headset instead of deferred.

---

## 1. Target UX (Half-Life: Alyx style)

- **Weapon wheel** — hold **A on the weapon hand**. Every carried weapon fans out in a circle centred on that
  controller. Move the hand toward one; it highlights (with a haptic tick as it crosses each entry). Release A
  to switch to the highlighted weapon; release with the hand still centred to cancel.
- **Item wheel** — hold **A on the off hand** (the hand that isn't the weapon hand). Same interaction over the
  carried *items* instead of weapons; release selects it as the active item (`SelectedItem`).
- **Active item** — the current `SelectedItem` rides the off hand the way the weapon rides the weapon hand
  (phase 4), and the **off hand's trigger activates it**. The off-hand trigger does nothing today (`UpdateButtons`
  only honours trigger/trackpad on the weapon hand — `VRPlayerInput.cpp:300-302`), so this repurposes a free control.

`A` is unbound on both hands in the shipped defaults (`LauncherSettings.h:154-157`), so both wheels have a free
button. `B` is already the menu toggle on the menu-pointer hand (`VRPlayerInput.cpp:259-263`); A vs B don't collide.

---

## 2. Verified levers (grounded in the current tree)

All confirmed by reading the code this session — do not re-derive:

- **Inventory chain is a walkable native linked list.** `pawn->Inventory()` (`UActor.h:582`) is the head;
  each node's `->Inventory()` (inherited by `UInventory` from `UActor`) is the next. Terminates at null.
- **Classification is a C++ cast.** `UObject::TryCast<UWeapon>(inv)` splits weapons from other inventory.
  `UWeapon : UInventory : UActor` (`UActor.h:856,925`).
- **Per-entry data is all accessor-reachable:** `PlayerViewMesh()` (`UActor.h:890`), `Mesh()`, `Icon()`
  (`:869`, a `UTexture*`), `InventoryGroup()` (`:870`), `bActivatable()` (`:900`), `PlayerViewOffset()`,
  `DrawScale()`.
- **Selection state is directly writable:** `pawn->PendingWeapon()` (`UActor.h:1960`), `pawn->Weapon()`
  (`:1992`), `pawn->SelectedItem()` (`:1969`).
- **Script functions are callable by object.** `CallEvent(UObject*, name, args)` (`VM/ScriptCall.h:39-40`) —
  this is exactly what `Engine::ExecCommand` dispatches through (`Engine.cpp:1795`). So we can invoke a specific
  weapon's/item's script method directly, not only a by-name exec that re-derives its own target.
- **Haptics:** `engine->vr->Haptic(hand, amplitude, duration=0)` — a zero duration is one runtime-minimum tick,
  "a wheel entry crossed" (`VRSubsystem.h:157-160`, already used by `UpdateFireHaptics`).
- **World-space 3D drawing per eye already exists twice over:** the phase-4 weapon override in
  `RenderSubsystem::DrawActor` (`RenderCanvas.cpp:152-235`) temporarily rewrites an actor's
  `Location`/`Rotation`/`DrawScale`, draws its mesh via `VisibleMesh::DrawMesh` into the VR eye frame, then
  restores it; and `DrawVRHands` (`RenderSubsystem.cpp:533-610`) builds a VR eye scene node with
  `BuildVREyeView` and draws unlit `Draw3DLine` geometry against the live camera. The wheel reuses both patterns.
- **Instances and tick order:** `engine->vrInput` (`VRPlayerInput`) and `engine->vrHands` (`VRHands`) are owned
  by `Engine` (`Engine.h:183-184`). Input ticks from `Engine::UpdateInput` before the level ticks; hands tick
  from `Engine::Run` after `PlayerCalcView` and before render (`Engine.cpp:217-223`). The renderer reads
  `engine->vrHands` directly.

---

## 3. Architecture — where the wheel lives

**Recommendation: a new `VRWheel` class owned by `Engine` (`engine->vrWheel`), mirroring `VRHands` exactly.**
It holds the wheel state (open/closed, which hand, the entry list, the highlighted index, the captured centre
pose), is *driven* from `VRPlayerInput::Tick`, and is *read* by `RenderSubsystem`. This is the same input/render
split phase 3–4 already use, and it keeps the inventory-walking and script-call logic out of both the input file
(which is about axes/keys) and the render file (which is about pixels).

State the class owns:

```
struct Entry { GCRoot<UInventory> Item; vec3 SlotForward; /* on the wheel plane */ };
bool Open;              // is a wheel currently up
int  Hand;              // which controller opened it (weapon hand => weapons, off hand => items)
Array<Entry> Entries;   // rebuilt each frame the wheel is open (inventory can change)
int  Highlighted;       // index into Entries, or -1 for "centred / cancel"
vec3 CenterPose...;     // controller grip position captured at open, the wheel's origin
```

`Entry::Item` is a `GCRoot` for the same reason `VRHands::Contact::Actor` is (`VRHands.h:76-79`): it is held
across frames and a carried item *can* be destroyed mid-wheel (picked-up-then-consumed, level edge cases);
selecting into freed memory would crash. Root while referenced, drop on close. Provide `Reset()` (forget
everything without acting) for map changes, called from wherever `vrHands->Reset()` is.

Why not fold it into `VRPlayerInput`: the renderer would then reach into an input class for geometry, and the
input class would grow a second responsibility. A peer of `VRHands` is the established shape.

---

## 4. Implementation, in landing order

Split into **6a (weapon wheel)** and **6b (item wheel + active item)** — 6a proves the whole mechanism
(open/layout/highlight/haptic/commit/render); 6b is mostly the same code pointed at items plus the active-item
attachment. The checklist calls phase 6 the largest build; these are the natural seams.

### 4a. Open / close (input)

Handle **A** as a hardcoded intercept in `VRPlayerInput::UpdateButtons`, the same way B/Escape and the
weapon-hand trigger are special-cased there (`VRPlayerInput.cpp:259-302`) — *before* the generic
`engine->InputEvent(ButtonToKey(...))` at the bottom, with a `continue` so A never also goes out as a Joy key.
Leave A's `ButtonCommands` slot empty and reserved (mirrors how the B/menu toggle needs no ini binding).

- On A **press**: if a pawn exists, no menu is up, and the pawn has inventory of the relevant kind, open the
  wheel for that hand — capture the controller grip position as the centre, build the entry list, haptic tick.
- On A **release**: commit (§4d) and close.
- Gate: never open while `engine->console->bNoDrawWorld()` (pause menu). If a menu opens while a wheel is up,
  force-close without committing (parallel to `ReleaseStickAction` being called on menu open, `Tick:205`).

Rationale for hardcoding rather than a bindable command: the wheel isn't a game exec/alias, it's a native VR
mode with its own render and gating. Routing it through the keybinding path would buy nothing and risk a game
alias swallowing it.

Add one launcher toggle only if wanted later; default hardcode A → weapon wheel (weapon hand) / item wheel
(off hand). Off hand = `1 - VRPlayerInput::WeaponHandIndex()`.

### 4b. Build the entry list (VRWheel, each open frame)

Walk `pawn->Inventory()` → `->Inventory()` to the end. For the **weapon wheel**, keep nodes where
`TryCast<UWeapon>` succeeds. For the **item wheel**, keep non-weapon inventory with **`bActivatable()` true** —
**verified as the correct predicate (§9):** the game's own `Inventory.SelectNext()` filters the item cycle on
exactly `bActivatable`, and `Inventory.Activate()` no-ops unless `bActivatable`, so a non-activatable item on the
wheel would be unselectable-then-inert anyway. `bDisplayableInv` is a *separate* flag meaning "show in the HUD
bar" — not what we want here. Rebuild every frame the wheel is open so a pickup/consume mid-wheel is reflected.
Cap the count defensively (e.g. 16) so a pathological inventory can't blow the layout; UE1 games never approach that.

### 4c. Layout, highlight, haptic (VRWheel)

- **Plane:** a disc facing the player, centred on the captured controller position, normal = toward the head
  (`engine->vr->GetHead()` / camera). Radius from a new setting `WheelRadiusCm` (world units via
  `MetersToUnrealUnits`).
- **Slots:** distribute `Entries.size()` evenly around the circle (top-centre = first). Each entry gets a
  `SlotForward` direction on the plane.
- **Highlight = hand displacement**, matching the checklist's "move the hand to one and release":
  `delta = currentGripPos - center`, projected onto the wheel plane. If `|delta| < WheelSelectDeadzoneCm`,
  `Highlighted = -1` (centred → cancel). Otherwise pick the entry whose `SlotForward` is closest to `delta`'s
  direction (max dot product). This is robust and needs no absolute controller calibration — only the delta
  from where the hand was when the wheel opened. (Tilt/aim-ray selection is the alternative; displacement is
  the specified UX and the safer default.)
- **Haptic:** when `Highlighted` changes to a new valid entry, `Haptic(Hand, 0.5f)` — one tick per crossing,
  exactly the pattern `UpdateFireHaptics` already uses.

### 4d. Commit on release — resolves the flagged InventoryGroup risk

The checklist's open risk: *"`SwitchWeapon` selects by `InventoryGroup`, so two weapons sharing a group are
indistinguishable through that path."* We sidestep it entirely by never selecting by group — we already hold
the exact `UWeapon*`/`UInventory*` object, so set the target directly. **The script contracts below are now
decompile-verified against BOTH Unreal Gold v226 and Unreal Tournament GOTY; see §9.**

**Weapon — mirror the tail of stock `PlayerPawn.SwitchWeapon`, substituting our object for its by-group lookup**
(UT99's exact ordering — set pending first, revert on refusal — which is also safe for Gold since `PutDown` is
async, §9):
```
if (Highlighted < 0) { close; return; }                 // cancel
UWeapon* W = entry;
if (W == pawn->Weapon()) { close; return; }              // already held → no-op
if (pawn->Weapon() == nullptr) {
    pawn->PendingWeapon() = W;
    CallEvent(pawn, "ChangedWeapon");                    // no current weapon: bring W up now
} else {
    pawn->PendingWeapon() = W;
    // Weapon.PutDown() returns bool and may refuse (mid-anim); on refusal, abort the switch.
    if (!CallEvent(pawn->Weapon(), "PutDown").ToType<bool>())   // (however the ExpressionValue bool is read)
        pawn->PendingWeapon() = nullptr;
}
```
Three corrections the decompile forced versus the first draft of this plan:
- The function is **`ChangedWeapon`** (past tense, `Pawn.uc:601`), *not* `ChangeWeapon`.
- **`PutDown()` returns `bool`** and the pending weapon is only committed when it returns true — this is exactly
  what `SwitchWeapon` (`PlayerPawn.uc:1375-1376`) does, and PutDown genuinely can decline / defer.
- **The switch is asynchronous.** `PutDown()` returns synchronously (true) but only *initiates* an animated
  put-down: `Idle`→`GotoState('DownWeapon')`, whose latent `FinishAnim()` then calls `Pawn.ChangedWeapon()` a
  few ticks later, which reads `PendingWeapon`, swaps `Weapon`, and `RaiseUp`s it. So setting `PendingWeapon`
  right after `PutDown()` returns still wins the race (ChangedWeapon hasn't run yet) — the same ordering the
  keyboard path relies on. Nothing in C++ needs to wait for or drive the animation; the state machine finishes it.

Because we set `PendingWeapon` to the exact `UWeapon*`, `WeaponChange(byte InventoryGroup)` (`Weapon.uc`, the
by-group walk `SwitchWeapon` normally uses) is never called — **the InventoryGroup ambiguity is gone, at the
engine level, with no fallback needed.** (A stray discovery: `PlayerPawn.GetWeapon(class<Weapon>)` selects by
class and would also be unambiguous, but `ExecCommand`'s string→arg conversion can't build a `class` argument
(`Engine.cpp:1779-1789` handles only byte/int/bool/float/string/name), so it isn't drivable that way — the
direct-object path above is both cleaner and the only reachable one.)

**Item — direct, no script call needed to select:**
```
if (Highlighted < 0) { close; return; }
pawn->SelectedItem() = entry;      // plain writable object pointer (UActor.h:1969)
```
Verified against `PlayerPawn.PrevItem`/`ActivateItem`, which read and write `SelectedItem` as a bare pointer.
No ambiguity, no async, nothing to drive.

### 4e. Render the wheel (RenderSubsystem, per eye)

New `RenderSubsystem::DrawVRWheel()`, called from `DrawGameFrame` next to `DrawVRHands`
(`RenderSubsystem.cpp:122`), gated on `CurrentVREye && engine->vrWheel && engine->vrWheel->Open`. Two proven
mechanisms, pick per entry type:

- **Weapons → 3D meshes** (the HL:Alyx look; render cost already accepted in the research notes). Reuse the
  phase-4 override: for each entry, save `Location`/`Rotation`/`DrawScale` **and `Mesh()`**, set
  `Mesh() = PlayerViewMesh()` (carried-but-unheld weapons may not have their view mesh assigned to `Mesh()`),
  place at the slot transform, `DrawScale *= WheelEntryScalePercent%`, draw via `VisibleMesh::DrawMesh` into the
  eye frame, restore. The highlighted entry draws larger/brighter. (Which mesh reads best — `PlayerViewMesh` vs
  third-person `Mesh`/`PickupViewMesh` — is an in-headset tuning call.)
- **Items → `Icon` billboards.** Every inventory item has an `Icon` (`UActor.h:869`) — that's what the 2D HUD
  shows, so it's the representation players recognise, and items have no meaningful first-person mesh. Draw each
  as a camera-facing textured quad at its slot (the `DrawTile`/tile machinery already renders `UTexture` to a
  quad; place it in world space like the menu/HUD planes). Simpler and cheaper than meshes.
- **Highlight ring / centre marker:** unlit `Draw3DLine` circles, straight out of the `DrawVRHands` playbook.
- Optional: entry name text via the existing `DrawText`, if legibility needs it — defer until seen in-headset.

### 4f. Gating while a wheel is open (input)

While `vrWheel->Open`, in `VRPlayerInput::Tick`:

- **Skip `UpdateAim`** (and thus the aim/view split) — the gun isn't being aimed, it's on the wheel; leave
  `ViewRotation` on the anchor. Skip `UpdateFireHaptics`.
- **Suppress the wheel hand's fire controls** — trigger and trackpad on that hand must not fire while you're
  picking. Cheapest: in `UpdateButtons`, when the wheel is open, don't emit Fire/AltFire for the wheel hand.
- **Locomotion stays live.** Movement and turn are on the thumbsticks, not A, and freezing them would feel
  worse than HL:Alyx (which lets you keep moving). Leave `UpdateMovement`/`UpdateTurning`/jump-crouch alone.
  Revisit only if in-headset testing shows the wheel hand's stick fights selection.

### 4g. Active item on the off hand (6b)

Two small pieces once the item wheel exists:

- **Draw the current `SelectedItem` on the off hand**, the phase-4 way. `DrawActor`'s override
  (`RenderCanvas.cpp:168`) currently fires for `IsPlayerViewWeapon(actor)` on the weapon hand; add a parallel
  path that, for the off hand, places `pawn->SelectedItem()`'s mesh on the off-hand pose. Reuse the same
  offset/scale knobs pattern (probably its own `ItemForward/Right/Up/…Offset` + `ItemScalePercent`). Not every
  item has a held mesh — fall back to just the `Icon` floating at the hand, or nothing, when there's no mesh.
- **Off-hand trigger activates it.** Intercept the off-hand trigger in `UpdateButtons` (it's currently
  suppressed there) → `CallEvent(pawn->SelectedItem(), "Activate")` when `SelectedItem` is set. **Verified (§9):**
  this is exactly what `PlayerPawn.ActivateItem` does (`if (SelectedItem != None) SelectedItem.Activate();`), and
  `Inventory.Activate()` is a nullary function that self-guards on `bActivatable`, so a non-activatable item is a
  safe no-op — we don't even need to re-check the flag before calling.

---

## 5. Settings additions (`LauncherSettings.h`, `VRSettingsPage.*`)

Keep the wheel-open button hardcoded (A). Add tuning knobs alongside the existing weapon knobs (§`LauncherSettings.h:110-129`):

- `int WheelRadiusCm` (default ~20)
- `int WheelSelectDeadzoneCm` (default ~4) — hand travel before a slot is chosen; below it = cancel
- `int WheelEntryScalePercent` (default reuse/around the weapon 500) — mesh size on the wheel
- `int ItemScalePercent` + `Item{Forward,Right,Up}OffsetCm` / `Item{Pitch,Yaw,Roll}OffsetDegrees` (6b, active-item placement)

Wire each into `VRSettingsPage` next to the weapon rows (the page already has the labelled-row pattern for
every VR knob). Item-wheel inclusion predicate (§4b) can start as a hardcoded `bActivatable` and only graduate
to a setting if the games disagree.

---

## 6. Risks, edge cases, and things that must be null-safe

- **Script-call contract (§4d, §4g)** — **decompile-verified for BOTH Unreal Gold v226 and Unreal Tournament
  GOTY (§9); no longer an unknown.** `ChangedWeapon`/`PutDown`(bool, async)/`Activate` behaviour is pinned in
  both. Only residual is confirming the mid-fire PutDown case in-headset (a feel check, not a contract check).
- **Selecting the held weapon / already-active item** — guard to a no-op (don't re-`PutDown` the current weapon).
- **Empty inventory / no pawn / spectator** — don't open; `TryCast<UPlayerPawn>` gate like every other VR path.
- **GC across frames** — `GCRoot` the entries (§3); a `bDeleteMe` entry must drop out of the list on the next
  rebuild rather than be drawn or selected.
- **Map change** — `vrWheel->Reset()` wherever `vrHands->Reset()` runs, so a wheel can't hold pointers into a
  torn-down level. Note the standing **Bugtracker** item ("loaded game crashes after a few seconds") is a
  pre-existing, separate defect; phase-6 code must be null-safe regardless but does not need to fix it.
- **Save/load** — the wheel is transient UI holding no game state; nothing to serialise. Just ensure it's
  closed/reset around a load.
- **Two-handed conflict** — both wheels can't sensibly be open at once; opening one while the other is up should
  close the first (single `Open`+`Hand` state enforces this for free).

---

## 7. Testing plan (headset available this session)

**Test installs (both should be exercised — only Unreal Gold has been used so far):**
- `build/Unreal Gold` — Unreal Gold v226, the folder the user launches from (`SE_AUTOLAUNCH=0`).
- `build/Unreal Tournament GOTY` — fresh GOG UT99 GOTY install added 2026-07-19. Weapon *combos* and a richer
  inventory (translocator, etc.) make it the better stress test for the wheels; it's also where the
  shared-`InventoryGroup` acceptance case (§4d step 3) actually occurs. The debugger/game binaries aren't copied
  into its `System/` yet — run tools as `./SurrealDebugger "build/Unreal Tournament GOTY"` from `build/`, or copy
  the binary in per the `build_copy_binary` workflow before an in-headset run.

Automated, headset-less (fast, first): drive the inventory walk and the `PendingWeapon`/`SelectedItem` writes +
`CallEvent` commit from a temporary desktop hook (the same forcing trick phases 1–2 used) and dump the resulting
`pawn->Weapon()` / `SelectedItem()` to **stderr** (not `LogMessage` — an unattended run killed by `timeout`
writes no log; research notes §"Automated testing"). This proves the switch logic and the group-ambiguity fix
without a headset.

In-headset (this is the phase where feel actually gets closed — **use the 4-minute / 240s timeout for these**,
per CLAUDE.md, the user exercises it and quits manually):

1. Open each wheel, confirm entries appear at the hand and highlight tracks hand movement with a tick per crossing.
2. Commit each — correct weapon comes up / correct item becomes active; cancel (centred release) does nothing.
3. Two weapons sharing an `InventoryGroup` (e.g. UT combos) both selectable — the acceptance test for §4d.
4. Fire is suppressed while the wheel is up; locomotion still works; pause menu force-closes the wheel.
5. Active item rides the off hand and its trigger activates it (6b).

---

## 8. Suggested commit seams

1. `VRWheel` skeleton + open/close intercept + entry-list build (no render, stderr dump) — headless-verifiable.
2. Commit logic (`PendingWeapon`/`PutDown`/`ChangedWeapon`; `SelectedItem`) — headless-verifiable, closes §4d risk.
3. Layout + highlight + haptic + `DrawVRWheel` (weapon meshes) — first in-headset pass = **6a done**.
4. Item wheel (icons) + active-item-on-off-hand + off-hand-trigger activate — **6b done**.
5. Launcher knobs + in-headset tuning pass.

---

## 9. Decompile findings — weapon-switch & item-activate contracts

Verified 2026-07-19 by decompiling the `Engine` package of **both Unreal Gold v226 and Unreal Tournament GOTY**
with `SurrealDebugger` (`export scripts Engine`, per the research-notes / `debug_runtime_workflow` recipe; both
exports deleted afterward). Line numbers below are Unreal Gold's export unless noted. These are the script
contracts the phase-6 commit path (§4d, §4g) rides on. **The two games agree on every contract; the only
difference is a cosmetic ordering in `SwitchWeapon` (below), which the §4d pseudocode already follows.**

**Weapon switch — `PlayerPawn.SwitchWeapon` (`PlayerPawn.uc:1349`), the pattern to mirror:**
```
if ( Weapon == None ) { PendingWeapon = newWeapon; ChangedWeapon(); }
else if ( (Weapon != newWeapon) && Weapon.PutDown() ) PendingWeapon = newWeapon;
```
`newWeapon` comes from `Inventory.WeaponChange(F)` — the **by-`InventoryGroup`** walk (`Weapon.uc`,
`if (InventoryGroup == F) ...`, confirmed identical in both games) that is the source of the flagged ambiguity.
We replace that lookup with our held `UWeapon*` and keep the rest verbatim.

**UT99 GOTY writes the same tail with the pending set *first* and reverted on refusal** (`PlayerPawn.uc:1349`):
```
if ( Weapon == None ) { PendingWeapon = newWeapon; ChangedWeapon(); }
else if ( Weapon != newWeapon ) { PendingWeapon = newWeapon; if ( !Weapon.PutDown() ) PendingWeapon = None; }
```
Semantically identical to Gold's (PutDown is async, so the pending assignment can't be clobbered by ChangedWeapon
before it's read either way). §4d uses this UT99 form because it's safe for both.

- **`ChangedWeapon`** (past tense) is the correct function name (`Pawn.uc:601`). It reads `PendingWeapon`, sets
  `Weapon = PendingWeapon`, calls `Weapon.RaiseUp(OldWeapon)`, clears `PendingWeapon`. Used for the
  no-current-weapon branch.
- **`Weapon.PutDown()` returns `bool`** and is per-state (`Weapon.uc`): `Idle`→`GotoState('DownWeapon'); return
  true`, `Active`→goes down or sets `bChangeWeapon`, `DownWeapon`→`return true`, plus a base
  `bChangeWeapon = true; return true`. It can defer, so the pending assignment is genuinely gated on the return.
- **Asynchronous.** `DownWeapon`'s Begin label — `TweenDown(); FinishAnim(); Pawn(Owner).ChangedWeapon();` — runs
  the swap only after the latent down-animation finishes (several ticks later). So `PutDown()` returns first, we
  set `PendingWeapon`, and `ChangedWeapon` later reads it. The keyboard path relies on this exact race; our C++
  does nothing special — no waiting, no per-frame driving, the state machine completes it.
- **`GetWeapon(class<Weapon>)`** (`PlayerPawn.uc:1379`) selects by class and would be unambiguous, but its arg is
  a `class` that `ExecCommand` cannot construct from a string (`Engine.cpp:1779-1789` — byte/int/bool/float/
  string/name only). Not usable via `ExecCommand`; the direct-object path is the only reachable one, and it's cleaner.

**Item select/activate:**
- **`SelectedItem`** is a bare writable pointer; `PlayerPawn.PrevItem`/`ActivateItem` read and assign it directly.
  Setting it to our held `UInventory*` *is* the selection — no ambiguity, no script call, no async.
- **`PlayerPawn.ActivateItem` (`PlayerPawn.uc:1423`):** `if (SelectedItem != None) SelectedItem.Activate();` — so
  the off-hand trigger → `CallEvent(SelectedItem, "Activate")` is exact.
- **`Inventory.Activate()` (`Inventory.uc:327`)** is nullary and self-guards: `if (bActivatable) { ...
  GoToState('Activated'); }`. Calling it on a non-activatable item is a safe no-op.
- **Item-wheel predicate = `bActivatable`.** `Inventory.SelectNext()` (`Inventory.uc`) filters the item cycle on
  precisely `bActivatable`, and `Activate()` guards on it too — so that flag, not `bDisplayableInv` ("show in HUD
  bar"), is the right membership test. Settles the §4b tuning question up front.

**UT99 GOTY item path is byte-for-byte equivalent:** `ActivateItem` = `if (SelectedItem != None)
SelectedItem.Activate();`; `Inventory.Activate()` self-guards on `bActivatable`; `SelectNext()` filters on
`bActivatable`; `SelectedItem` written directly. Only cosmetic difference: UT99 wraps the activate/select
`ClientMessage` in `if (M_Activated/M_Selected != "")` — irrelevant to us.

**Net effect:** every script interaction the commit path needs is confirmed for **both** Unreal Gold and UT99
GOTY; the InventoryGroup ambiguity is fully avoidable at the engine level with no fallback and no remaining
decompile chore. What's left is in-headset feel only.
