# Installation

## Supported platforms

OS               |  Architecture   | Compiler
-----------------|-----------------|----------------
Ubuntu 24.04 LTS | Aarch64, X86_64 | clang-21/libc++, gcc-15/libstdc++

## Setup development tools

- Install system utilities 
  ```bash
  sudo apt install software-properties-common build-essential pkg-config gpg wget ca-certificates \
  git-lfs curl ninja-build ccache doxygen graphviz linux-generic python3-full python3-dev \
  python-is-python3 pybind11-dev python3-wheel python3-setuptools python3-build pipx avahi-daemon \
  avahi-utils iproute2 iputils-ping net-tools iftop htop nvtop patch
  ```

- Install build tools
  - [CMake](./howto/install_cmake.md) and helpers
  - [LLVM toolchain](./howto/install_llvm.md)
  - [GCC toolchain](./howto/install_gcc.md)

- Install support libraries for 3D graphics and GUIs 
  ```bash
  sudo apt install libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev \
  libxkbcommon-dev libwayland-dev wayland-protocols
  ```

## Configure and build

Using Clang toolchain:

```bash
git clone git@github.com:cvilas/grape
cmake --preset clang
cmake --build build/clang --target all examples check
```

Using GCC:

```bash
git clone git@github.com:cvilas/grape
cmake --preset native
cmake --build build/native --target all examples check
```
