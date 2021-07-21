# Unreal Tournament Engine Reimplementation

The goal of this project is to reimplement enough of the original Unreal Engine to make the original Unreal Tournament maps playable.

![facingworlds](https://user-images.githubusercontent.com/5136903/125014285-afc65580-e06d-11eb-80c0-0a1992a7d0ff.jpg)

Currently it can boot enough of unrealscript that the menus can render themselves, as illustrated by the above screenshot. :)

## Windows build instructions

Open UTEngine.sln with Visual Studio 2019. There are no external third party dependencies.

## Linux build instructions

Use CMake to build the project. You need to the dev packages for the following things: cmake, g++, pthreads, dl, alsa (libasound2) and xlib. I can't remember their exact names anymore and they may vary from distro to distro anyway. In general, if you get an include error that looks like it is trying to include something external, then you are probably missing the dev package for that library. :)
