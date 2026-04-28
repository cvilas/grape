# Cross-compiling for aarch64 target on x86-64 host

## Limitations

- No python bindings support. This is too complex for little gain
- No rpi_camera support

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

- Install gcc and sysroot (glibc and kernel headers for aarch64) at `/usr/aarch64-linux-gnu/` 
  ```bash
  GCC_VERSION=16
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

- Install llvm arm64 _runtime_ libraries (_dev_ libraries will overwrite X86 dev libraries and break the system)
  ```bash
  sudo apt-get install -y libc++1:arm64 libc++abi1:arm64
  ```

- Install arm64 system libraries

  External dependencies require platform graphics libraries. Install their arm64 multiarch variants:

  ```bash
  sudo apt-get install -y \
    libpipewire-0.3-dev:arm64 libdrm-dev:arm64 libgbm-dev:arm64 \
    libwayland-dev:arm64 libegl1-mesa-dev:arm64 libxkbcommon-dev:arm64 \
    libdecor-0-dev:arm64 libthai-dev:arm64 \
    libx11-dev:arm64 libxext-dev:arm64 libxi-dev:arm64 \
    libcamera-dev:arm64
  ```

## Design notes

- Don't set `CMAKE_SYSROOT`. Instead the toolchain sets `CMAKE_FIND_ROOT_PATH` to 
  `/usr/aarch64-linux-gnu`, which is the sysroot provided by `gcc-15-aarch64-linux-gnu`. This 
  avoids gcc and clang from picking up host libraries. The sysroot role is split between 
  `CMAKE_FIND_ROOT_PATH` (which handles CMake's library/package discovery), and 
  `--gcc-toolchain=/usr` (which handles CRT object discovery by `lld`).



