#=================================================================================================
# Copyright (C) 2023 GRAPE Contributors
#=================================================================================================

# macOS native toolchain file.
# Use AppleClang and default SDK include/link ordering to avoid libc++ wrapper
# header failures when third-party projects inject SDK include paths.

if(NOT APPLE)
  message(FATAL_ERROR "toolchain_osx.cmake can only be used on macOS")
endif()

set(CMAKE_C_COMPILER "/usr/bin/clang" CACHE FILEPATH "C compiler" FORCE)
set(CMAKE_CXX_COMPILER "/usr/bin/clang++" CACHE FILEPATH "CXX compiler" FORCE)
set(CMAKE_CROSSCOMPILING FALSE)

# Keep libc++ explicit but rely on Apple toolchain defaults for header/lib paths.
set(CMAKE_CXX_FLAGS "-stdlib=libc++")


