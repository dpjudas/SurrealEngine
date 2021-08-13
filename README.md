# Unreal Tournament Engine Reimplementation

The goal of this project is to reimplement enough of the original Unreal Engine to make the Unreal Tournament (UT99) maps playable.

![facingworlds](https://user-images.githubusercontent.com/5136903/125014285-afc65580-e06d-11eb-80c0-0a1992a7d0ff.jpg)

## Current status

The engine can load and render the maps. The Unrealscript VM is almost feature complete - only arrays and network conditional execution are not implemented yet.

It will attempt to load all level actors and initialize the map. However, while the menus and the HUD will appear, there are still many native functions not implemented yet. It is also quite possible some events aren't firing as they should. You will therefore see exceptions shown if you interact with them and that is where the project is at.

## Command line parameters

`UTEngine [--url=<mapname>] [--engineversion=X] [Path to game folder]`

If no game folder is specified the engine will try search the registry (Windows only) for the registry keys Epic originally set. If no URL is specified it will use the default URL in the ini file (per default the intro map). The --engineversion argument overrides the internal version detected by the engine and should only be used for debugging purposes.

At the time of this writing the game itself can either be Unreal Tournament (436 version only) or Unreal Gold. Unreal Gold is significantly more buggy at the moment though (only the intro map really works).

## Windows build instructions

Use CMake to build the project. There are no other external third party dependencies.

## Linux build instructions

Use CMake to build the project. You need to the dev packages for the following things: cmake, g++, pthreads, dl, alsa (libasound2) and xlib. I can't remember their exact names anymore and they may vary from distro to distro anyway. In general, if you get an include error that looks like it is trying to include something external, then you are probably missing the dev package for that library. :)
