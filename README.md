# Surreal Engine

The goal of this project is to reimplement enough of the original Unreal Engine to make the Unreal Tournament (UT99) maps playable.

![facingworlds](https://user-images.githubusercontent.com/5136903/125014285-afc65580-e06d-11eb-80c0-0a1992a7d0ff.jpg)

## Current status

The engine can load and render the maps. The Unrealscript VM is almost feature complete - only arrays and network conditional execution are not implemented yet.

It will attempt to load all level actors and initialize the map. However, while the menus and the HUD will appear, there are still many native functions not implemented yet. It is also quite possible some events aren't firing as they should. You will therefore see exceptions shown if you interact with them and that is where the project is at.

At the time of this writing, SurrealEngine can detect the following UE1 games:

* Unreal Tournament (v436, v451b, v469(b, c, d))
* Unreal Gold (v226, v227(i, j))
* Deus Ex (v1112fm)

Klingon Honor Guard has its executable's SHA1 sum missing, so it cannot be detected, although the relevant entries for it are there.

From the list above, only Unreal Tournament v436 is in a relatively playable state. Running any other game (and UT versions) can and will result in crashes. Unreal Gold v226 also runs but is significantly more buggy compared to UT (only the intro map really works)

### Unreal Tournament v436

The game launches, menu options will work and botmatches can be played, however the bots won't have any AI, and some maps will have some functionality missing (like DM-Morpheus will not have the "X leading the match" screens work).

## Command line parameters

`SurrealEngine [--url=<mapname>] [--engineversion=X] [Path to game folder]`

You can simply copy paste the SurrealEngine executable inside the System folder of your UE1 game of choice, and run it from there. If no game folder is specified, and the executable isn't in a System folder, the engine will search the registry (Windows only) for the registry keys Epic originally set. If no URL is specified it will use the default URL in the ini file (per default the intro map). The --engineversion argument overrides the internal version detected by the engine and should only be used for debugging purposes.

## Windows build instructions

Use CMake to build the project. There are no other external third party dependencies.

## Linux build instructions

Use CMake to build the project.

You'll need the dev packages for the following things:

* cmake
* g++
* pthreads
* dl
* alsa (libasound2)

If you have SDL2 installed on your system, SurrealEngine will utilise it for its windowing functionalities and native Wayland support (with `SDL_VIDEODRIVER=wayland`). Otherwise, it will fall back to using X11, which requires the following packages:

* XRandR
* Xlib

Note that these packages won't always have the exact names given above, as it can change from distro to distro. In general, if you get an include error that looks like it is trying to include something external, then you are probably missing the dev package for that library. :)
