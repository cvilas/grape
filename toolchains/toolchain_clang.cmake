#=================================================================================================
# Copyright (C) 2023 GRAPE Contributors
#=================================================================================================

set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_CXX_FLAGS "-stdlib=libc++")
set(CMAKE_CROSSCOMPILING FALSE)

# Direct the linker to the arch-qualified paths to avoid multiarch symlink conflicts
cmake_host_system_information(RESULT _host_arch QUERY OS_PLATFORM)
set(CMAKE_EXE_LINKER_FLAGS    "-stdlib=libc++ -lc++abi -fuse-ld=lld -L/usr/lib/${_host_arch}-linux-gnu")
set(CMAKE_SHARED_LINKER_FLAGS "-fuse-ld=lld -L/usr/lib/${_host_arch}-linux-gnu")
set(CMAKE_MODULE_LINKER_FLAGS "-fuse-ld=lld")

