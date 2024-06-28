#=================================================================================================
# Copyright (C) 2023 GRAPE Contributors
#=================================================================================================

set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_CXX_FLAGS "-stdlib=libc++")
set(CMAKE_EXE_LINKER_FLAGS "-stdlib=libc++ -lc++abi")
set(CMAKE_CROSSCOMPILING FALSE)
