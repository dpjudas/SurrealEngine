# Unreal Tournament Engine Reimplementation

The goal of this project is to reimplement enough of the original Unreal Engine to make the Unreal Tournament (UT99) maps playable.

![facingworlds](https://user-images.githubusercontent.com/5136903/125014285-afc65580-e06d-11eb-80c0-0a1992a7d0ff.jpg)

## Current status

The engine can load and render the maps. The Unrealscript VM is almost feature complete - only latent functions, arrays and network conditional execution is not implemented yet.

It will attempt to load all level actors and initialize the map. However, while the menus and the HUD will appear, there are still many native functions not implemented yet. It is also quite possible some events aren't firing as they should. You will therefore see exceptions shown if you interact with them and that is where the project is at.

You will have to edit UTEngine/Engine.cpp and correct the path at the top of the file to where you have Unreal Tournament installed. It also has to be patch 436. 469 will not work currently. All of this will of course not stay hardcoded if/when the project will actually run a full deathmatch game.

Also maybe worth mentioning: the Linux mouse handling is pretty bad. On my Linux Mint it gave me quite a lot of mouse lag. If someone knows how to fix that easily I'm very open to pull requests. Otherwise I'll probably fix it myself once the project reaches a playable state.

## Windows build instructions

Open UTEngine.sln with Visual Studio 2019. There are no external third party dependencies.

## Linux build instructions

Use CMake to build the project. You need to the dev packages for the following things: cmake, g++, pthreads, dl, alsa (libasound2) and xlib. I can't remember their exact names anymore and they may vary from distro to distro anyway. In general, if you get an include error that looks like it is trying to include something external, then you are probably missing the dev package for that library. :)
