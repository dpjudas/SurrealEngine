# Building Surreal Engine

## Windows

Use CMake to build the project. A recent version of Visual Studio and MSVC compiler that supports C++20 is required. Any other compilers are not supported.

## Linux

Use CMake to build the project. Development versions of the following packages are needed:

* cmake
* g++
* pthreads
* dl
* alsa (libasound2)
* waylandpp (Optional - C++ bindings for Wayland protocols, used on SurrealWidgets Wayland backend)
* SDL2 or SDL3 (Optional - Used on SurrealWidgets SDL2/SDL3 backends)

> [!NOTE]
> If both SDL2 and SDL3 are present on the system, SDL3 will take precedence.

> [!NOTE]
> These packages won't always have the exact names given above, as it can change from distro to distro. In general, if you get an include error that looks like it is trying to include something external, then you are probably missing the dev package for that library. :)

### Installing the prerequisite packages

#### Ubuntu

    # apt install cmake g++ libasound-dev libopenal-dev libdbus-1-dev libsdl3-dev libxkbcommon-dev waylandpp-dev

#### Arch Linux

    # pacman -S libx11 gcc git cmake sdl3 alsa-lib waylandpp

#### Fedora

    # dnf install libX11 libX11-devel libxkbcommon libxkbcommon-devel SDL3-devel openal-soft-devel alsa-lib-devel waylandpp waylandpp-devel

### After installing prerequisites

Once you've installed all prerequisites, enter these commands in the given order from the folder you want to clone the repo to:

    git clone https://github.com/dpjudas/SurrealEngine.git
    cd SurrealEngine
    mkdir build
    cd build
    cmake -DCMAKE_BUILD_TYPE=Release ..
    make -j 16

When compilation is successfully finished, `build` folder should contain these 3 executables: `SurrealEngine`, `SurrealEditor` and `SurrealDebugger`