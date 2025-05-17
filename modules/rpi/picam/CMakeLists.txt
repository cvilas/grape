# =================================================================================================
# Copyright (C) 2025 grapecam contributors
# =================================================================================================

declare_module(
  NAME picam
  DEPENDS_ON_MODULES "log"
  DEPENDS_ON_EXTERNAL_PROJECTS "SDL3")

# TODO: Detect whether libcamera was built with gcc/libstdc++ or clang/libc++. Don't allow
# build if toolchain doesn't match.
#if(NOT CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
#  message(WARNING "${MODULE_NAME}: Requires GCC/libstdc++. Bypassing module")
#  return()
#endif()

set(LIBCAMERA_VERSION_REQUIRED "0.5")
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBCAMERA QUIET IMPORTED_TARGET libcamera>=${LIBCAMERA_VERSION_REQUIRED})

if(NOT LIBCAMERA_FOUND)
  message(WARNING "${MODULE_NAME}: Requires libcamera, which is not installed. Bypassing module.")
  return()
endif()

message(STATUS "libcamera: Using version: ${LIBCAMERA_VERSION} (${LIBCAMERA_LINK_LIBRARIES})")

add_subdirectory(hello)
add_subdirectory(list)
add_subdirectory(view)
