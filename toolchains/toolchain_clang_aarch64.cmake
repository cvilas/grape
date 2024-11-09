#=================================================================================================
# Copyright (C) 2024 GRAPE Contributors
#=================================================================================================

# Clang cross-compiling toolchain to build for Aarch64 target on X86 host
# TODO(vilas): Fix this toolchain setup
# - Libraries for 'target' arch must be provided and their paths specified. Unlike gcc toolchain,
#   there are apparently no apt installable packages for aarch binaries on x86. Either copy them 
#   from your target machine or cross-build llvm for the target and provide path to it
# - I am going farther with static builds than shared builds. With the later, linker finds wrong 
#   libraries (host rather than target)

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)
set(BUILD_SHARED_LIBS OFF) # TODO(vilas): Linker fails if 'ON'

# C++ compiler toolchain paths
set(LLVM_DIR "/usr/lib/llvm-20")
set(LLVM_INCLUDE_DIR "${LLVM_DIR}/include/c++/v1")
set(LLVM_LIBRARY_DIR "${LLVM_DIR}/lib") # TODO(vilas): should be location of libraries for 'target' arch 

# C++ compiler flags
set(CROSS_TARGET "aarch64-linux-gnu")
set(CMAKE_C_COMPILER   "${LLVM_DIR}/bin/clang")
set(CMAKE_CXX_COMPILER "${LLVM_DIR}/bin/clang++")
set(CROSS_COMPILE_FLAGS "-target ${CROSS_TARGET} -stdlib=libc++ -I${LLVM_INCLUDE_DIR}")
set(CMAKE_C_FLAGS_INIT   "${CROSS_COMPILE_FLAGS}" CACHE STRING "Initial C compiler flags")
set(CMAKE_CXX_FLAGS_INIT "${CROSS_COMPILE_FLAGS}" CACHE STRING "Initial C++ compiler flags")
set(CMAKE_EXE_LINKER_FLAGS_INIT "-fuse-ld=lld --target=${CROSS_TARGET} -L${LLVM_LIBRARY_DIR} -lc++ -lc++abi" CACHE STRING "Initial linker flags")

# Rust toolchain settings
set(RUSTC_TRIPLE aarch64-unknown-linux-gnu)
set(ENV{RUSTFLAGS} "-Clinker=clang -Clink-arg=--target=aarch64-linux-gnu -Car=llvm-ar")

# Uncomment to debug find_package
#set(CMAKE_FIND_DEBUG_MODE 1)

# Enable static analysis for host build only
set(ENABLE_LINTER OFF)
set(ENABLE_FORMATTER OFF)

# Look in specific places for all the libraries, and only look there
list(APPEND CMAKE_FIND_ROOT_PATH "/usr/${CROSS_TARGET};${CMAKE_INSTALL_PREFIX}")
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

set(CMAKE_CROSSCOMPILING_EMULATOR qemu-aarch64-static -L /usr/${CROSS_TARGET})
