# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this is

Surreal Engine is a from-scratch reimplementation of **Unreal Engine 1**, focused on making Unreal (Gold) and Unreal Tournament (UT99) playable. It loads the original games' packages and runs their UnrealScript. See `Docs/Status.md` for what currently works (only UT99 v436 and Unreal Gold v226 are considered playable; other games/versions crash).

## Building

CMake, C++20 (MSVC on Windows, g++ on Linux — other compilers unsupported):

```
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j 16
```

Produces three executables in `build/`: `SurrealEngine` (the game), `SurrealEditor`, and `SurrealDebugger`. See `Docs/Building.md` for the full list of Linux dev-package prerequisites (alsa, libXi, glib2, fontconfig, optional SDL2/3 and waylandpp, plus python3 for the OpenXR build).

`Configure.js` is an alternative build definition for the `cppbuild` tool; CMake is the primary path.

Notable CMake option: `ENABLE_OPENXR` (default ON) compiles in VR support and requires Python3 at build time. It defines `USE_OPENXR` and links `openxr_loader`.

### Running

`SurrealEngine [--url=<mapname>] [--engineversion=X] [path to game folder]`. With no arguments it launches a ZWidget launcher where game folders are configured. VR is opt-in via a launcher setting even when compiled in.

### Tests

There is **no unit-test suite**. Files named `*Test` under `Collision/` (e.g. `TraceTest`, `OverlapTest`) are collision-query implementations, not tests. Verify changes by running the game against a real UE1 install.

**VR tests that require user interaction** (shooting, swimming, moving around, anything the user has to do in-headset) must run with a **4-minute (240s) timeout** — the user needs several minutes to exercise a scenario, and will quit the game manually when done testing before the timeout. Do not use short timeouts for these. (Automated headset-less plumbing checks that need no user action can stay short.)

## Architecture

All engine code compiles into one static library, `SurrealCommon`, which the three executables link against. Each executable is just a thin `Main*.cpp` + `*App` entry point (`MainGame.cpp`/`GameApp`, `MainEditor.cpp`/`EditorApp`, `MainDebugger.cpp`/`DebuggerApp`).

`Engine` (`SurrealEngine/Engine.{h,cpp}`) is the central object: it is the `GameWindowHost`, owns the main loop (`Run`), and drives map loading, save/load, and `ClientTravel` between maps.

The UE1 model is reproduced across several layers that must be understood together:

- **`Package/`** — loads the original `.u`/`.unr`/`.utx`/etc. package files. `PackageManager` resolves and caches packages; `PackageStream` deserializes objects. `IniFile` handles the games' `.ini` config. Game/version detection lives in `UE1GameDatabase.*`.
- **`UObject/`** — the UE1 object model in C++. Classes are prefixed **`U`** (`UObject`, `UClass`, `UActor`, `ULevel`, `UMesh`, `UTexture`, `UFont`, `USound`...). `PropertyOffsets.*` maps native C++ fields onto the script-visible property layout. `U227*` files support Unreal Gold 227 extensions.
- **`VM/`** — the UnrealScript bytecode virtual machine. `Frame` is an execution frame, `Bytecode`/`ExpressionEvaluator` interpret script, `NativeFunc` dispatches to native functions.
- **`Native/`** — C++ implementations of UnrealScript native functions, one file per script class, prefixed **`N`** (`NActor`, `NPawn`, `NCanvas`...). Each exposes a `RegisterFunctions()` that binds native functions into the VM. When adding native support for a script class, this is where it goes.
- **`Render/`** — high-level scene rendering via `RenderSubsystem`: BSP traversal/clipping (`BspClipper`), and per-object `Visible*` builders (`VisibleActor`, `VisibleMesh`, `VisibleBrush`, `VisiblePortal`...). This produces draw work for the render device.
- **`RenderDevice/`** — low-level GPU backends behind `RenderDevice`: `Vulkan/` and `D3D11/`. There is no OpenGL backend. Uses the `SurrealGPU` library for GPU abstraction.
- **`Collision/`** — BSP collision split into `TopLevel/` (trace/overlap queries against the world) and `BottomLevel/`.
- **Supporting subsystems**: `Audio/` (mixer + OpenAL/alsa), `Video/` + `SurrealVideo/` (ffmpeg-based cutscene playback, built as the `SurrealVideo` shared lib), `Light/`, `Math/` (vec/mat/quaternion/rotator, `coords`), `GC/` (garbage collector for UObjects), `UI/` (Launcher/Editor/ErrorWindow, all built on `SurrealWidgets`).

### Coordinate system & units

World axes are **X forward, Y right, Z up**. 1 Unreal unit = 1.905 cm (`MetersToUnrealUnits = 52.4934` in `VR/VRSubsystem.h`). Anything crossing between Unreal units and real-world metres (VR poses, world-anchored VR UI) must convert through this.

### VR (`VR/`)

Head tracking + stereo rendering via OpenXR, plus hand aiming, a wrist HUD tablet, a weapon/item wheel and touch pickup. Built on the `vr_renderer` branch; **read [`Docs/VR.md`](Docs/VR.md) before touching any of it** — it covers the architecture, the aim/view split, every launcher setting and the known limitations. Two invariants worth repeating here:

- `VRSubsystem` is an abstract base whose virtuals all default to inert/no-op behavior, so callers only branch on `IsActive()`, never on whether VR was compiled in.
- `VRSubsystem::Create(enabled, renderScalePercent)` returns a `NullVRSubsystem` when disabled, when built without `USE_OPENXR`, or when no OpenXR runtime is reachable — **VR failing to initialize must never stop the desktop game from starting.**

## Bundled libraries (separate CMake subprojects)

- **`SurrealGPU/`** — GPU graphics library.
- **`SurrealWidgets/`** — cross-platform C++ UI framework (a.k.a. ZWidget); backs the launcher, editor, and error windows. Has X11/Wayland/SDL backends; the Wayland backend is known to be buggy.
- **`SurrealVideo/`** — ffmpeg-derived video decoding, built as a shared library.

`SurrealEngine.pk3` holds engine-provided assets (shaders, UI resources) packaged into the build.
