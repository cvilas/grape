#=================================================================================================
# Copyright (C) 2018 GRAPE Contributors
# MIT License
#=================================================================================================

# GCC cross-compiling toolchain to build for Aarch64 target on X86 host

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)
set(CMAKE_CROSSCOMPILING TRUE)

set(CMAKE_C_COMPILER aarch64-linux-gnu-gcc)
set(CMAKE_CXX_COMPILER aarch64-linux-gnu-g++)

# Set Rust-specific variables
set(RUSTC_TRIPLE aarch64-unknown-linux-gnu)
set(ENV{RUSTFLAGS} "-Clinker=aarch64-linux-gnu-gcc -Car=aarch64-linux-gnu-ar")

# Uncomment to debug find_package
#set(CMAKE_FIND_DEBUG_MODE 1)

# Enable static analysis for host build only
set(ENABLE_LINTER OFF)
set(ENABLE_FORMATTER OFF)
set(ENABLE_CACHE OFF)

# Look in specific places for all the libraries, and only look there
list(APPEND CMAKE_FIND_ROOT_PATH "/usr/aarch64-linux-gnu;${CMAKE_INSTALL_PREFIX}")
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

set(CMAKE_CROSSCOMPILING_EMULATOR qemu-aarch64-static -L /usr/aarch64-linux-gnu)
