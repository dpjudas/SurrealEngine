# Surreal Engine

The goal of this project is to reimplement enough of the original Unreal Engine to make the Unreal Tournament (UT99) maps playable.

![facingworlds](https://user-images.githubusercontent.com/5136903/125014285-afc65580-e06d-11eb-80c0-0a1992a7d0ff.jpg)

## Requirements

* Original copies of the UE1 games you want to run
* Windows 10+ or a modern Linux distro
* A Vulkan capable graphics card

## Current status

The engine can load and render the maps. The Unrealscript VM is almost feature complete - only arrays and network conditional execution are not implemented yet.

It will attempt to load all level actors and initialize the map. However, while the menus and the HUD will appear, there are still many native functions not implemented yet. It is also quite possible some events aren't firing as they should. You will therefore see exceptions shown if you interact with them and that is where the project is at.

At the time of this writing, SurrealEngine can detect the following UE1 games:

* Unreal Tournament (v436, v451b, v469(a, b, c, d))
* Unreal (v200, v209, v220, v224v, v225f, v226f)
* Unreal Gold (v226b, v227(i, j, k_11))
* Deus Ex (v1002f, v1112fm)
* Klingon Honor Guard (219)
* NERF Arena Blast (v300)
* TNN Outdoors Pro Hunter (v200)
* Rune Classic (v1.10)
* Clive Barker's Undying (v420)
* Tactical-Ops: Assault on Terror (v3.4.0 and v3.5.0 - both running under UT436 and UT469 engines)
* Wheel of Time (v333)

From the list above, only Unreal Tournament v436 and Unreal Gold v226 is in a relatively playable state. Running any other game (and UT versions) can and will result in crashes.

### Unreal Tournament v436

The game launches, menu options will work and botmatches can be played, however the bots will barely have any AI (they move around sometimes and retaliate upon being attacked), and some maps will have some functionality missing (like DM-Morpheus will not have the "X leading the match" screens work).

### Unreal Gold v226

The game launches, menu options will work most of the time. Single player maps can be played, as well as botmatches. The AI will behave more or less the same as how they behave in UT. Objects from Return to Na Pali have their models appear broken.

### Deus Ex v1112fm

Only the intro flyby works. No keyboard or mouse inputs will be detected, as Deus Ex handles them differently than Unreal/UT. 

## Command line parameters

`SurrealEngine [--url=<mapname>] [--engineversion=X] [Path to game folder]`

You can simply copy paste the SurrealEngine executable inside the System folder of your UE1 game of choice, and run it from there. If no game folder is specified, and the executable isn't in a System folder, the engine will search the registry (Windows only) for the registry keys Epic originally set. If no URL is specified it will use the default URL in the ini file (per default the intro map). The --engineversion argument overrides the internal version detected by the engine and should only be used for debugging purposes.

## Windows build instructions

Use CMake to build the project. A recent version of Visual Studio, and MSVC compiler that supports C++17 is required.

On Windows, SDL2 is an optional dependency that you need to supply locally yourself (download the Visual C++ version of SDL2, extract it somewhere and point to that folder in CMake settings). Supplying SDL2 will allow you to use it as an alternative windowing system.

Other than that there are no other external third party dependencies.

## Linux build instructions

Use CMake to build the project. You're gonna need the development versions of the following packages:

* cmake
* g++
* pthreads
* dl
* alsa (libasound2)
* SDL2
* waylandpp (optional) (C++ bindings for Wayland, used on ZWidget Wayland backend)

On Linux, SDL2 is required, as SurrealEngine will utilise it for its windowing functionalities and native Wayland support (with `SDL_VIDEODRIVER=wayland`).

Note that these packages won't always have the exact names given above, as it can change from distro to distro. In general, if you get an include error that looks like it is trying to include something external, then you are probably missing the dev package for that library. :)

### Ubuntu

    # apt install cmake g++ libasound-dev libopenal-dev libdbus-1-dev libsdl2-dev libxkbcommon-dev waylandpp-dev

Once you've installed all of the prerequisites, enter these commands in the given order from the folder you want to clone the repo to:

    git clone https://github.com/dpjudas/SurrealEngine.git
    cd SurrealEngine
    mkdir build
    cd build
    cmake -DCMAKE_BUILD_TYPE=Release ..
    make -j 16

When compilation is successfully finished, `build` folder should contain these 3 executables: `SurrealEngine`, `SurrealEditor` and `SurrealDebugger`
