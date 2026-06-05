#=================================================================================================
# Copyright (C) 2024 GRAPE Contributors
#=================================================================================================

# Clang cross-compiling toolchain to build for Aarch64 target on X86 host
# See cross_compile_aarch64.md for setup instructions

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)
set(CMAKE_CROSSCOMPILING TRUE)

# Do NOT set CMAKE_SYSROOT: Ubuntu cross packages use absolute paths in their linker scripts
# (e.g. GROUP(/usr/aarch64-linux-gnu/lib/libm.so.6)); CMAKE_SYSROOT would cause lld to
# double-prefix those paths. Use CMAKE_FIND_ROOT_PATH for library discovery instead.

set(CMAKE_C_COMPILER   clang)
set(CMAKE_CXX_COMPILER clang++)

set(CROSS_TARGET "aarch64-linux-gnu")

# Locate the libc++ headers from clang's own resource directory.
execute_process(
  COMMAND clang++ --print-resource-dir
  OUTPUT_VARIABLE _clang_resource_dir
  OUTPUT_STRIP_TRAILING_WHITESPACE)
# resource dir is <llvm>/lib/clang/<ver>; headers are at <llvm>/include/c++/v1
cmake_path(GET _clang_resource_dir PARENT_PATH _clang_lib_clang)
cmake_path(GET _clang_lib_clang    PARENT_PATH _clang_lib)
cmake_path(GET _clang_lib          PARENT_PATH _clang_root)
set(LIBCXX_INCLUDE_DIR "${_clang_root}/include/c++/v1")

# Use the versioned soname directly (-l:libc++.so.1) rather than -lc++ to avoid symlinks to wrong arch.
set(LIBCXX_LIB_DIR "/usr/lib/aarch64-linux-gnu")

# Tell clang/lld where to find aarch64 GCC CRT objects (crtbeginS.o, libgcc, etc.)
# Clang searches <prefix>/lib/gcc-cross/<triple>/<ver>/ on Ubuntu/Debian.
set(GCC_TOOLCHAIN "--gcc-toolchain=/usr")

# -nostdinc++ / -nostdlib++: suppress clang's automatic injection of libc++ headers/libs via full paths into the compiler and linker command lines.
# Supply everything explicitly using arch-qualified paths.
set(CMAKE_C_FLAGS_INIT "--target=${CROSS_TARGET} ${GCC_TOOLCHAIN}" CACHE STRING "Initial C compiler flags")
set(CMAKE_CXX_FLAGS_INIT "--target=${CROSS_TARGET} ${GCC_TOOLCHAIN} -nostdinc++ -isystem ${LIBCXX_INCLUDE_DIR}" CACHE STRING "Initial C++ compiler flags")
set(CMAKE_EXE_LINKER_FLAGS_INIT "-fuse-ld=lld --target=${CROSS_TARGET} ${GCC_TOOLCHAIN} -nostdlib++ ${LIBCXX_LIB_DIR}/libc++.so.1 ${LIBCXX_LIB_DIR}/libc++abi.so.1" CACHE STRING "Initial linker flags")
set(CMAKE_SHARED_LINKER_FLAGS_INIT "-fuse-ld=lld --target=${CROSS_TARGET} ${GCC_TOOLCHAIN} -nostdlib++" CACHE STRING "Initial shared linker flags")

# Rust toolchain settings
set(RUSTC_TRIPLE aarch64-unknown-linux-gnu)
set(ENV{RUSTFLAGS} "-Clinker=clang -Clink-arg=--target=aarch64-linux-gnu -Car=llvm-ar")

# Disable host-only flags
set(ENABLE_LINTER OFF)
set(ENABLE_FORMATTER OFF)

# Uncomment to debug find_package
#set(CMAKE_FIND_DEBUG_MODE 1)

# Search sysroot and install prefix; never use host programs
list(APPEND CMAKE_FIND_ROOT_PATH "/usr/aarch64-linux-gnu" "${CMAKE_INSTALL_PREFIX}")
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

set(CMAKE_CROSSCOMPILING_EMULATOR qemu-aarch64-static -L /usr/aarch64-linux-gnu)
