# Cross-compiling for aarch64 with Clang on x86-64

## Limitations

- No python bindings support. This is too complex for little gain
- No rpi_camera support

## TODO

- [ ] Add x-build to X86 CI

## Overview

The build system supports cross-compiling for `aarch64-linux-gnu` from an x86-64 host 

```bash
# cross build using llvm toolchain (toolchain_clang_aarch64.cmake)
cmake --preset llvm-cross
cmake --build build/llvm-cross

# cross build using gcc toolchain (toolchain_gcc_aarch64.cmake)
cmake --preset gcc-cross
cmake --build build/gcc-cross
```

## One-time host setup

- Enable multiarch support for arm64
  ```bash
  sudo dpkg --add-architecture arm64
  sudo apt-get update
  ```

- Install gcc and sysroot at `/usr/aarch64-linux-gnu/` (glibc and kernel headers) for aarch64 
  ```bash
  GCC_VERSION=15
  sudo apt-get install -y gcc-$GCC_VERSION-aarch64-linux-gnu g++-$GCC_VERSION-aarch64-linux-gnu

  PRIORITY=$((${GCC_VERSION%%.*} * 10))
  sudo update-alternatives --remove-all aarch64-linux-gnu-gcc 2>/dev/null || true
  sudo update-alternatives --install /usr/bin/aarch64-linux-gnu-gcc aarch64-linux-gnu-gcc /usr/bin/aarch64-linux-gnu-gcc-$GCC_VERSION $PRIORITY \
  --slave /usr/bin/aarch64-linux-gnu-g++ aarch64-linux-gnu-g++ /usr/bin/aarch64-linux-gnu-g++-$GCC_VERSION \
  --slave /usr/bin/aarch64-linux-gnu-gcov aarch64-linux-gnu-gcov /usr/bin/aarch64-linux-gnu-gcov-$GCC_VERSION
  ```

- Install QEMU for running cross-built binaries
  ```bash
  sudo apt-get install -y qemu-user-static binfmt-support
  ```

- Install llvm arm64 _runtime_ libraries (_dev_ libraries will overwrite X86 libraries)
  ```bash
  sudo apt-get install -y libc++1:arm64 libc++abi1:arm64
  ```

- Install arm64 system libraries

  SDL3 and other external dependencies require graphics and audio libraries. Install their arm64 multiarch variants:

  ```bash
  sudo apt-get install -y \
    libpipewire-0.3-dev:arm64 libdrm-dev:arm64 libgbm-dev:arm64 \
    libwayland-dev:arm64 libegl1-mesa-dev:arm64 libxkbcommon-dev:arm64 \
    libdecor-0-dev:arm64 libthai-dev:arm64 \
    libx11-dev:arm64 libxext-dev:arm64 libxi-dev:arm64 \
    libcamera-dev:arm64
  ```

## Design decisions and pitfalls

### Don't set CMAKE_SYSROOT

Instead `CMAKE_FIND_ROOT_PATH` is set to `/usr/aarch64-linux-gnu`, which is the sysroot provided by
`gcc-15-aarch64-linux-gnu`. Avoids gcc and clang from picking up host libraries

The sysroot role is split between `CMAKE_FIND_ROOT_PATH` (which handles CMake's library/package
discovery), and `--gcc-toolchain=/usr` (which handles CRT object discovery by `lld`).

### Python bindings

Python bindings built with nanobind target the **host** Python ABI and cannot be
cross-compiled. `gbs/07_modules_py.cmake` detects `CMAKE_CROSSCOMPILING` and defines
`define_module_pybinding()` as a macro that calls `return()`. In CMake, `return()` inside a
macro executes in the **caller's** scope, so the entire `py/CMakeLists.txt` (including
`add_subdirectory(tests)`) is skipped without needing per-module guards.


