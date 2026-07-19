![SEBANNER](Resources/surreal-engine-banner.png)

# Welcome to Surreal Engine!

Surreal Engine is a project that aims to reimplement Unreal Engine 1; currently focused on making Unreal (Gold) and Unreal Tournament (UT99) playable. The scope of this project might expand to cover more UE1 games in the future.

## Current Status

Please refer to [Status.md](Docs/Status.md) for the current status of Surreal Engine!

## System Requirements

* Original copies of the UE1 games you want to run
* Windows 10+ or a modern Linux distro
* A Direct3D 11 or Vulkan capable graphics card

## Building Surreal Engine

Please refer to [Building.md](Docs/Building.md) for details!

## VR

Surreal Engine can play the supported games in VR through OpenXR, developed and tested on a Valve Index.
It is compiled in by default and enabled from the launcher's VR tab. See [VR.md](Docs/VR.md).

## Downloads

[Nightly builds are available on the Releases section](https://github.com/dpjudas/SurrealEngine/releases/tag/nightly).

Additionally, Surreal Engine is available on following Linux distributions:

* Arch: [AUR](https://aur.archlinux.org/packages/surrealengine-git)
* Nix: [Package Search](https://search.nixos.org/packages?channel=unstable&show=surreal-engine) | [Quickstart](https://github.com/NixOS/nixpkgs/pull/337069)

## How to Play

* Run the `SurrealEngine` executable.
* Add the UE1 games you want in the Folders tab.
* Select the game you want to play in Games tab.
* Click "Play"!

## Discord Server

Visit us on Discord at https://discord.gg/5AEry4s

## Command Line Parameters

`SurrealEngine [--url=<mapname>] [--engineversion=X] [Path to game folder]`

If no game folder is specified, and the executable isn't in a System folder, the engine will search the registry (Windows only) for the registry keys Epic originally set.

If no URL is specified it will use the default URL in the ini file (per default the intro map).

The `--engineversion` argument overrides the internal version detected by the engine and should only be used for debugging purposes.
