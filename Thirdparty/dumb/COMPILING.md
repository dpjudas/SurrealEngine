# Compiling

## 1. CMake

### 1.1. Example

In libdumb project root, run:
```
mkdir -p build
cd build
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local -DBUILD_SHARED_LIBS:BOOL=ON -DBUILD_ALLEGRO4:BOOL=ON ..
make
make install
```

### 1.2. Steps

1. Create a new temporary build directory and cd into it
2. Run libdumb cmake file with cmake (eg. `cmake -DCMAKE_INSTALL_PREFIX=/install/dir -DBUILD_SHARED_LIBS:BOOL=ON -DCMAKE_BUILD_TYPE=Release path/to/dumb/cmake/dir`).
3. Run make (eg. just `make` or `mingw32-make` or something).
4. If needed, run make install.

### 1.3. Flags

* `CMAKE_INSTALL_PREFIX` sets the installation path prefix.
* `CMAKE_BUILD_TYPE` chooses the build type: `Release`, `Debug`, `RelWithDebInfo`, or `MinSizeRel`. Debug libraries will be named `libdumbd`, release libraries `libdumb`. Default is `Release`.
* `BUILD_SHARED_LIBS` selects whether cmake should build a dynamic (`ON`) or static (`OFF`) library. Default is `OFF`.
* `BUILD_ALLEGRO4` enables (`ON`) or disables (`OFF`) the optional Allegro 4 support. This requires Allegro 4 installed on the system. Default is `ON`.
* `BUILD_EXAMPLES` selects example binaries. These example binaries require argtable2 and SDL2 libraries. Default is `ON`.
* `USE_SSE` enables or disables SSE support. Default is `ON`.
* You may also need to tell cmake what kind of makefiles to create with the "-G" flag. Eg. for MSYS one would say something like `cmake -G "MSYS Makefiles" .`.

## 2. Visual Studio

TODO
