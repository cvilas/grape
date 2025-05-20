#=================================================================================================
# Copyright (C) 2023 GRAPE Contributors
#=================================================================================================

set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_CXX_FLAGS "-stdlib=libc++")
set(CMAKE_EXE_LINKER_FLAGS "-stdlib=libc++ -lc++abi")
set(CMAKE_CROSSCOMPILING FALSE)

# Additional configuration for MacOS
# LLVM must be installed: `brew install llvm`
# LLVM binaries should be in path: `export PATH="/opt/homebrew/opt/llvm/bin:$PATH"`
if(CMAKE_SYSTEM_NAME MATCHES "Darwin")
  execute_process(COMMAND xcrun --show-sdk-path OUTPUT_VARIABLE CMAKE_OSX_SYSROOT OUTPUT_STRIP_TRAILING_WHITESPACE)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -isysroot ${CMAKE_OSX_SYSROOT}")
endif()
