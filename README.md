# Surreal Engine

The goal of this project is to reimplement enough of the original Unreal Engine to make the Unreal Tournament (UT99) maps playable, and more UE1 games in the future.

![facingworlds](https://user-images.githubusercontent.com/5136903/125014285-afc65580-e06d-11eb-80c0-0a1992a7d0ff.jpg)

## Requirements

* Original copies of the UE1 games you want to run
* Windows 10+ or a modern Linux distro
* A DirectX 11 or Vulkan capable graphics card

## Current status

Please refer to [Status.md](Docs/Status.md) for the current status of Surreal Engine!

## Downloads

[Nightly builds are available on the Releases section](https://github.com/dpjudas/SurrealEngine/releases/tag/nightly). Just download the zip file for your OS, and either:

* extract everything to the `system` folder of the UE1 game you want to play and run the `SurrealEngine` executable.

or

* extract it to anywhere you want and provide the path of the UE1 game you want to play as a parameter (see "Command line parameters" section below).

Additionally, Surreal Engine is available on following Linux distributions:

* Arch: [AUR](https://aur.archlinux.org/packages/surrealengine-git)
* Nix: [Package Search](https://search.nixos.org/packages?channel=unstable&show=surreal-engine) | [Quickstart](https://github.com/NixOS/nixpkgs/pull/337069)

## Discord server

Visit us on Discord at https://discord.gg/5AEry4s

## Command line parameters

`SurrealEngine [--url=<mapname>] [--engineversion=X] [Path to game folder]`

If no game folder is specified, and the executable isn't in a System folder, the engine will search the registry (Windows only) for the registry keys Epic originally set.

If no URL is specified it will use the default URL in the ini file (per default the intro map).

The --engineversion argument overrides the internal version detected by the engine and should only be used for debugging purposes.


## Building Surreal Engine

### Windows

Use CMake to build the project. A recent version of Visual Studio, and MSVC compiler that supports C++20 is required.

On Windows, SDL2 is an optional dependency that you need to supply locally yourself (download the Visual C++ version of SDL2, extract it somewhere and point to that folder in CMake settings). Supplying SDL2 will allow you to use it as an alternative windowing system.

Other than that there are no other external third party dependencies.

### Linux

Use CMake to build the project. You're gonna need the development versions of the following packages:

* cmake
* g++
* pthreads
* dl
* alsa (libasound2)
* SDL2 (Optional - Used on ZWidget SDL2 backend)
* waylandpp (Optional - C++ bindings for Wayland, used on ZWidget Wayland backend)

> [!NOTE] 
> On some distros, SDL2 is replaced by SDL3, so you'll probably need the devel package for the **compat library** on these instead.

> [!NOTE] 
> These packages won't always have the exact names given above, as it can change from distro to distro. In general, if you get an include error that looks like it is trying to include something external, then you are probably missing the dev package for that library. :)

#### Installing the prerequisite packages

**Ubuntu**

    # apt install cmake g++ libasound-dev libopenal-dev libdbus-1-dev libsdl2-dev libxkbcommon-dev waylandpp-dev

**Arch Linux**

    # pacman -S libx11 gcc git cmake sdl2 alsa-lib waylandpp
    
**Fedora**

    # dnf install libX11 libX11-devel libxkbcommon libxkbcommon-devel SDL2-devel openal-soft-devel alsa-lib-devel waylandpp waylandpp-devel

#### After installing prerequisites

Once you've installed all of the prerequisites, enter these commands in the given order from the folder you want to clone the repo to:

    git clone https://github.com/dpjudas/SurrealEngine.git
    cd SurrealEngine
    mkdir build
    cd build
    cmake -DCMAKE_BUILD_TYPE=Release ..
    make -j 16

When compilation is successfully finished, `build` folder should contain these 3 executables: `SurrealEngine`, `SurrealEditor` and `SurrealDebugger`
