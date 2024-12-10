#=================================================================================================
# Copyright (C) 2024 GRAPE Contributors
#=================================================================================================

# TODO(vilas): Fix this toolchain
# Clang cross-compiling toolchain to build for Aarch64 target on X86 host

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)
set(CMAKE_CROSSCOMPILING TRUE)

set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)

set(CPP_TRIPLE aarch64-linux-gnu)
set(CMAKE_C_COMPILER_TARGET ${CPP_TRIPLE})
set(CMAKE_CXX_COMPILER_TARGET ${CPP_TRIPLE})
set(CMAKE_CXX_FLAGS "-stdlib=libc++")
set(CMAKE_EXE_LINKER_FLAGS "-stdlib=libc++ -lc++abi")

# Set additional flags for Clang
#set(CMAKE_SYSROOT /usr/aarch64-linux-gnu)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --target=aarch64-linux-gnu")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --target=aarch64-linux-gnu")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=lld")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fuse-ld=lld")

# Set Rust-specific variables
set(RUSTC_TRIPLE aarch64-unknown-linux-gnu)
set(ENV{RUSTFLAGS} "-Clinker=clang -Clink-arg=--target=aarch64-linux-gnu -Car=llvm-ar")

# Uncomment to debug find_package
#set(CMAKE_FIND_DEBUG_MODE 1)

# Enable static analysis for host build only
set(ENABLE_LINTER OFF)
set(ENABLE_FORMATTER OFF)

# Look in specific places for all the libraries, and only look there
list(APPEND CMAKE_FIND_ROOT_PATH "/usr/aarch64-linux-gnu;${CMAKE_INSTALL_PREFIX}")
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

set(CMAKE_CROSSCOMPILING_EMULATOR qemu-aarch64-static -L /usr/aarch64-linux-gnu)
