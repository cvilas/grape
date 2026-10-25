# README - grape/external

This directory defines an independent CMake project to configure and build external dependencies. 
Note that configuration is not shared with the main project in either direction, except those
parameters that are passed manually across the two projects. 

## Dear ImGui and ImPlot

Unmodified upstream archives live in [sources/](sources/). Small standalone
[CMake wrappers](wrappers/) build and install the libraries, headers, licenses,
and relocatable CMake packages.

### Using the packages

Declare `imgui` and/or `implot` in a module's `DEPENDS_ON_EXTERNAL_PROJECTS`.
The external build expands `implot -> imgui -> SDL3` and builds in dependency
order. Modules can then use:

```cmake
find_package(imgui ${IMGUI_VERSION_REQUIRED} EXACT REQUIRED CONFIG)
find_package(implot ${IMPLOT_VERSION_REQUIRED} EXACT REQUIRED CONFIG)
```

Pass the appropriate targets to the module's link-library arguments:

| Target | Contents |
|---|---|
| `imgui::imgui` | Core and `misc/cpp/imgui_stdlib` support |
| `imgui::sdl3` | SDL3 platform/input backend |
| `imgui::sdlrenderer3` | SDL_Renderer rendering backend |
| `imgui::sdlgpu3` | SDL_GPU rendering backend, including embedded upstream shaders |
| `implot::implot` | ImPlot; publicly links `imgui::imgui` |

Link `implot::implot`, `imgui::sdl3`, and **one** rendering backend in an application.
Both rendering libraries are installed; choose one per ImGui context at runtime.
ImPlot does not need a separate rendering backend. The ImGui package requires
SDL3 because it exports all three SDL3 backend targets.

### Build and configuration

For a standalone build:

```sh
cmake -S external -B build/gui-external -G Ninja \
  -DEXTERNAL_PROJECTS_LIST=implot \
  -DBUILD_SHARED_LIBS=ON \
  -DCMAKE_INSTALL_LIBDIR=lib \
  -DCMAKE_INSTALL_PREFIX="$PWD/build/gui-external/deploy"
cmake --build build/gui-external --parallel
```

### Package tests

After building/installing the packages:

```sh
cmake -S external/tests -B build/gui-package-tests -G Ninja \
  -DCMAKE_PREFIX_PATH="$PWD/build/gui-external/deploy"
cmake --build build/gui-package-tests --parallel
ctest --test-dir build/gui-package-tests --output-on-failure
```

The tests render ImPlot and a mesh exceeding 65,535 vertices using SDL's dummy
video/software renderer, exercise the C++ string adapter and demos, and link SDL_GPU backend 
entry points without requiring GPU hardware. They do not validate actual GPU rendering.
