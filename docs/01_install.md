# Installation

## Supported platforms

OS               |  Architecture   | Toolchain
-----------------|-----------------|----------------
Ubuntu 24.04 LTS | Aarch64, X86_64 | CMake-4, clang-21/libc++, gcc-15/libstdc++

## Setup development environment

- Install base system utilities and development tools
  ```bash
  chmod +x ./toolchains/install_base.sh
  ./toolchains/install_base.sh
  ```

- Install CMake
  
  If the required version is available from repositories:
  ```bash
  chmod +x ./toolchains/install_cmake.sh
  ./toolchains/install_cmake.sh
  ```
  Otherwise, [build from source](./howto/build_cmake.md)

- Install LLVM toolchain

  ```bash
  chmod +x ./toolchains/install_llvm.sh
  ./toolchains/install_llvm.sh
  ```

- Install GCC toolchain

  If the required version is available from repositories:
  ```bash
  chmod +x ./toolchains/install_gcc.sh
  ./toolchains/install_gcc.sh
  ```
  Otherwise, [build from source](./howto/build_gcc.md)

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
