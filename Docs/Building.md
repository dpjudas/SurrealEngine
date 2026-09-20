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

## Haiku

Use CMake to build the project. Haiku has no ALSA/OSS/D-Bus and no Vulkan driver, so
the build talks to audio and video through SDL2 instead, and the launcher defaults to
the OpenGL renderer rather than Vulkan.

Install the prerequisites from HaikuDepot / `pkgman`:

    pkgman install cmake sdl2_devel

`sdl3_devel` will also be picked up automatically if it's installed (and preferred over
SDL2 for windowing if both are present), but SDL2 alone is enough to build and run.

Then, from the folder you want to clone the repo to:

    git clone https://github.com/dpjudas/SurrealEngine.git
    cd SurrealEngine
    mkdir build
    cd build
    cmake -DCMAKE_BUILD_TYPE=Release ..
    make -j $(nproc)

> [!NOTE]
> Vulkan isn't available on Haiku, so the `Vulkan` render device option in the launcher
> won't work. Make sure `OpenGL` is selected under the Video settings page (this is the
> default on Haiku already, but double check if you're carrying over an existing
> `~/.config/SurrealEngine/Settings.json`).