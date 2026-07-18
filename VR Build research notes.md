# VR Build — research notes

Companion to `VR Build checklist.md`. The checklist says *what to build and in what order*; this file records *what was verified, how, and what is still unknown*, so a later session does not re-derive it.

Session date: 2026-07-17. Branch: `vr_renderer`.

---

## Verified facts (do not re-check)

**Joy1–Joy16 are stock Epic bindings, not leftovers from an earlier experiment.**
Checked four installs. Three (`~/GOG Games/Unreal Gold`, `~/Games/Heroic/Unreal Gold`, the Heroic gogdl copy) contain **no `SE-*.ini` files at all**, proving SurrealEngine never ran against them — and all three ship `Joy1=Fire` / `Joy2=Jump` / `Joy3=AltFire` / `Joy4=Duck` in both `DefUser.ini` and `User.ini`. UT99 GOTY ships the same. `build/Unreal Gold/System/DefUser.ini` is md5-identical (`7259148d…`) to the pristine GOG copy. Chain: Epic ships `DefUser.ini` → game creates `User.ini` → SurrealEngine copies to `SE-User.ini` (`Package/PackageManager.cpp:592-638`).
⇒ The comment at `VR/VRPlayerInput.cpp:118-119` ("Nothing binds these by default") is factually wrong.

**The first-person weapon is already correctly stereo-projected.**
`RenderSubsystem::DrawActor` (`Render/RenderCanvas.cpp:125`) renders into `MainFrame`; `RenderScene.cpp:32-36` gives `MainFrame` the VR eye's projection when `CurrentVREye` is set. The weapon's problem is *placement only* — the script's `CalcDrawOffset` positions it relative to the camera, welding it to the head. It is **not** part of the flat-2D-onto-the-eye problem.

**Only `PreRender`/`PostRender` are the flat-2D problem.**
`Render/RenderSubsystem.cpp:95-107` runs `PreRender()` / `RenderOverlays()` / `PostRender()` per eye. `PreRender`/`PostRender` paint console+HUD flat at zero depth. `RenderOverlays` draws the weapon through the 3D frame (above). ⇒ Suppressing the HUD does not cost you the weapon.

**The 2D UI is not fixed-pixel.**
`ResetCanvas` (`Render/RenderCanvas.cpp:33-35`) derives `uiscale` from target height; HUD lays out in a virtual 1024x768 (engine < 400) or 1280x960 space.

**The UI cannot be split into parts.** HUD and menus both emerge from the single `console->PostRender` script call.

**The VR menu plane machinery already works**, gated on `bNoDrawWorld()`: `DrawUICanvas()` (offscreen 1920x1440, `RenderDevice::VRMenuCanvasWidth/Height`), `DrawVRMenuPlane()` (world-space quad, correct stereo depth), `UpdateVRMenuLaser()` (laser cast + cursor mapping, handles both the UWindow script-menu path and the native `dxRootWindow` path).

**Aim and view are the same variable — desktop-only; corrected for VR in phase 5.**
`AdjustAim` is script (no native impl anywhere) and reads `ViewRotation`. `PlayerCalcView` returns `ViewRotation`; `Engine.cpp` assigns its result to `CameraLocation`/`CameraRotation`. Fire origin is `Owner.Location + FireOffset`, computed in script — unreachable from C++ without moving the pawn.
⇒ **In VR the view is already HMD-driven and independent of `ViewRotation`** (the head pose is composed on top of `CameraRotation` in `BuildVREyeView`), so "pointing the gun swings the camera" was the *desktop* reading. Phase 5 exploits this: it hijacks `ViewRotation` for aim each gameplay frame and restores `CameraRotation`/`ViewRotation`/body `Rotation` to a body-anchor yaw right after `PlayerCalcView`, so only the shot follows the hand. See the `vr_phase5_aim_split` memory and phase 5 in the checklist.

**On-foot movement is oriented by body `Rotation`, not `ViewRotation`** (verified against decompiled `Engine.PlayerPawn`). `PlayerWalking.PlayerMove` does `GetAxes(Rotation,X,Y,Z); NewAccel = aForward*X + aStrafe*Y` at the *top*, before `UpdateRotation` copies `ViewRotation` onto `Rotation` — so it uses the *previous* frame's body rotation. This is why the phase-5 aim hijack is safe for walking as long as body `Rotation` is restored to the anchor each frame. (`PlayerSwimming`/`PlayerFlying` do use `GetAxes(ViewRotation)` — those states currently move toward the aimed hand; noted, not yet addressed.)

**Missing OpenXR actions** (`VR/OpenXRSubsystem.cpp:178-225`): no grip pose (only `AimPoseAction` → `/input/aim/pose`), no trackpad, no `XR_ACTION_TYPE_VIBRATION_OUTPUT`. Index profile has **no** `trackpad/click` — only `trackpad/x`, `y`, `force`, `touch`.

**Joy key mapping is `IK_Joy1 + hand * ButtonCount + button`** (`VR/VRPlayerInput.cpp:174`). Appending to the `Button` enum shifts every *right-hand* key (the "add to the end only" comment only protects the left). At 9 buttons/hand it silently overruns `IK_Joy16`. No bounds check.

**Wheel levers exist:** `pawn->Inventory()` (walkable native chain), `Pawn.SelectedItem` (`PropertyOffsets.cpp:166`), `Inventory.InventoryGroup`, `engine->ExecCommand` reaches script exec functions (`SwitchWeapon`, `ActivateItem`, `NextItem`).

**Movement aliases are two commands each** (`DefUser.ini`):
```
Aliases[8]=(Command="Jump | Axis aUp Speed=+300.0",Alias=Jump)
Aliases[9]=(Command="Button bDuck | Axis aUp Speed=-300.0",Alias=Duck)
```
Setting `bPressedJump` directly skips the `aUp` half.

---

## Closed lead — `SetBool` on `bDuck` (resolved 2026-07-17, phase 2 unblocked)

**Answer: it is genuinely a byte property, the cast is formally UB, and it works anyway. Phase 2 may route crouch through the `Duck` alias.**

Decompiled Unreal Gold's `Engine.u` to settle it (`Pawn.uc:176-177`):
```
var input byte
	bZoom, bRun, bLook, bDuck, bSnapLevel,
```
So `bDuck` is a `UByteProperty`; `UActor.h:2007`'s `uint8_t&` accessor is right and the `SavedMove.bDuck` `BitfieldBool` accessor (`UActor.h:1447`) is a *different, genuinely-bool* property of the same name. `PlayerPawn.uc:78`'s `bPressedJump` is a real `var bool`, but nothing sets it through this path — the `Jump` alias half is an exec function, not a `Button`.

Why it survives, exactly:
- `UBoolProperty::SetBool` is **non-virtual** and `UBoolProperty` **declares no data members**, so `static_cast<UBoolProperty*>(byteProp)->SetBool(...)` just reads `DataOffset` out of the `UProperty` base. No vtable or layout hazard.
- `UClass.cpp:120-159` only assigns `BitfieldMask` on the bool branch. Byte properties keep the struct default `BitfieldMask = 1` (`PropertyOffsets.h:10`).
- So the write is `*(uint32_t*)ptr |= 1` on press, `&= ~1` on release (`UProperty.cpp:493-500`). Bit 0 lands in the target byte and the OR/AND preserves the other three bytes — `bDuck` goes 1/0 as intended, neighbours untouched.

**This is not a `bDuck` quirk — the entire button-input path depends on it.** Every UE1 input button (`bFire`, `bAltFire`, `bRun`, `bDuck`, …) is `input byte`, and all of them reach the pawn via `Engine.cpp:1445-1446`'s `SetBool`. Desktop fire and crouch have always gone through this cast.

Residual risks, neither observed:
- It is a 4-byte read-modify-write at a 1-byte property's offset. A byte property in the last 3 bytes of `PropertyData` would over-read/write. `bDuck` sits mid-`Pawn` with many properties after it, so it is safe today — but the hazard is real for any future byte property placed last.
- Misaligned 32-bit access; harmless on x86/ARM64, would trap on stricter targets.

⇒ Do not "fix" `SetBool` as a prerequisite for phase 2 — that would change desktop fire/crouch behaviour. If it gets hardened, it is a separate change with its own desktop regression pass.

**How this was verified (reusable):** `SurrealDebugger`'s console cannot be driven by piping stdin — it needs a pty, *and* it drops queued lines that arrive in one chunk (`DebuggerApp::GetInput` overwrites `cmdline` per line while `InCommandlet` is true). Drive it from a `pty.fork()` sending one command at a time, ~3s apart. `export scripts` with no package argument dies on `ObjectStream::ReadString: Invalid size in Crashsite2`; scope it: `export scripts Engine`. Output lands in `<gameRootFolder>/Engine/Classes/*.uc`.

---

## Automated testing without a headset — what works, what doesn't

**The ceiling: no OpenXR runtime is reachable without the Index.** The active runtime is SteamVR (`~/.config/openxr/1/active_runtime.json`), which needs the hardware. No Monado is installed, so there is no simulator driver to fake poses against. Every OpenXR call added on this branch is **compile-verified only** until the headset is back. `LauncherSettings.VR.Enabled` is `true` in `Settings.json`, so unattended runs do try OpenXR, fail to find a headset, and fall through to desktop — which at least keeps re-proving that VR failing to init never stops the desktop game.

**`SE_AUTOLAUNCH=<index>` skips the launcher** (`GameApp.cpp`, added this session). This is load-bearing for all automation: `LauncherWindow::ExecModal()` is otherwise unconditional — *no command line argument skips it*, including the game folder path — so without this the process just sits in the launcher's Wayland event loop forever and nothing can be tested. Index 0 is the game the launcher would have preselected. Full unattended run:

```
cd "build/Unreal Gold/System"
SE_AUTOLAUNCH=0 timeout 75 ./SurrealEngine --url=DmAriza "<gameRootFolder>" 2>&1
```

**Debug output must go to stderr, not `LogMessage`.** `Logger` keeps the whole log in memory and only writes `SE-Log-LastRun.txt` from `Engine::Run`'s clean exit — so a run ended by `timeout` (i.e. every unattended run) leaves **no log at all**, and `SE-Log-LastRun.txt` still holds whatever the last interactive run wrote. That file is a trap when reading results of an automated run. `fprintf(stderr, ...)` is unbuffered and survives the kill.

**Time in a hook must come from `timeElapsed`, not a frame count.** The game clock runs well behind wall clock on an unattended run; ~12s of game time needs a 40s+ `timeout`.

**Driving the debugger console non-interactively:** it needs a pty *and* paced input — `DebuggerApp::GetInput` overwrites `cmdline` per line while `InCommandlet` is true, so lines arriving in one chunk overwrite each other and only the last survives. Either `pty.fork()` sending one command every ~3s, or the subshell-with-sleeps form in the `debug_runtime_workflow` memory. `export scripts` with no package argument dies on `ObjectStream::ReadString: Invalid size in Crashsite2`; scope it (`export scripts Engine`). Output lands in `<gameRootFolder>/<Package>/Classes/*.uc` — delete when done.

**What a headset-less run can still prove:** anything reachable from the desktop input path. The phase 2 alias mechanism was fully verified this way (see the checklist), and `ApplyKeybindings` was verified by temporarily forcing it to run with VR inactive. `static_assert`s can be verified by deliberately breaking them.

---

## Decisions settled this session

- **Weapon-wheel render cost is accepted.** Drawing every carried weapon's `PlayerViewMesh` while the wheel is open is fine; no measurement gate.
- **Implementation order** is the phase order in `VR Build checklist.md`: input foundation → jump/crouch → HUD tablet → weapon placement → aim split → wheel.
- **VR claims the stock Joy keys** rather than moving off them (2026-07-17). `VRPlayerInput::ApplyKeybindings` overwrites Joy1–16 at startup from a table. The ini is never neutral — Epic ships gamepad-shaped defaults that every install inherits — so reading it would mean every VR player starts with left grip on Jump and the right hand scattered across `SwitchWeapon`. Accepted cost: ini Joy bindings do nothing while VR is on. Side benefit: the `Button` enum can be appended to freely, since bindings are regenerated from it rather than inherited.
- **Free Joy keys are the synthetic-key budget.** 7 buttons × 2 hands = Joy1–14; Joy15/16 are jump/crouch's press/release bookkeeping. The range is now exactly full, and a `static_assert` catches the next append.

## Decisions still open

- HUD tablet: selectable item, or always on the forearm?
- Tablet transparency: `DrawUICanvas` clears opaque — needs a transparent clear + blended plane draw.
- Phase 5: how much fire-origin mismatch (shots leaving the chest while the muzzle is at the hand) is acceptable? Decide **before** starting the view/aim split.
- `SwitchWeapon` selects by `InventoryGroup` — two weapons sharing a group are indistinguishable through that path. No workaround identified.

---

## Corrections made mid-session (guard against regression)

Two claims were asserted and later disproved by checking. If either resurfaces, it is wrong:

1. ~~"The weapon lands flat on each eye at zero depth."~~ It is correctly 3D-projected; only its placement is wrong.
2. ~~"`SE-User.ini`'s Joy bindings were created by an earlier step."~~ They are stock Epic defaults.
