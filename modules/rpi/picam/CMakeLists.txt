# =================================================================================================
# Copyright (C) 2025 grapecam contributors
# =================================================================================================

declare_module(
  NAME picam
  DEPENDS_ON_MODULES "log"
  DEPENDS_ON_EXTERNAL_PROJECTS "SDL3")

set(LIBCAMERA_VERSION_REQUIRED "0.5")
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBCAMERA QUIET IMPORTED_TARGET libcamera>=${LIBCAMERA_VERSION_REQUIRED})

if(NOT LIBCAMERA_FOUND)
  message(WARNING "${MODULE_NAME}: Requires libcamera, which is not installed. Bypassing module.")
  return()
endif()
message(
  STATUS
    "${MODULE_NAME}: Using libcamera version: ${LIBCAMERA_VERSION} (${LIBCAMERA_LINK_LIBRARIES})")

# Extract path to libcamera library
set(LIBCAMERA_SO_PATH "libcamera_not_found")
foreach(LIB ${LIBCAMERA_LINK_LIBRARIES})
  string(FIND "${LIB}" "libcamera.so" FOUND_POS)
  if(NOT ${FOUND_POS} EQUAL -1)
    set(LIBCAMERA_SO_PATH "${LIB}")
    break()
  endif()
endforeach()

# Determine what standard library we are using
detect_cxx_stdlib()
if(USING_LIBSTDCXX)
  set(STDLIB_PATTERN "libstdc++")
elseif(USING_LIBCPP)
  set(STDLIB_PATTERN "libc++")
else()
  set(STDLIB_PATTERN "unknown_std_lib")
endif()

# Detect what standard library libcamera was built with
execute_process(
  COMMAND bash -c "ldd ${LIBCAMERA_SO_PATH} | grep -oE 'libc\\+\\+|libstdc\\+\\+' | sort -u"
  OUTPUT_VARIABLE LIBCAMERA_STDLIB
  OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)

# We must use the same standard library that libcamera uses, otherwise will have linker errors due
# to ABI incompatibility
if(NOT LIBCAMERA_STDLIB STREQUAL STDLIB_PATTERN)
  message(WARNING "${MODULE_NAME}: We are using C++ standard library '${STDLIB_PATTERN}' but \
  libcamera requires '${LIBCAMERA_STDLIB}'. Both are incompatible. Bypassing module")
  return()
endif()

add_subdirectory(hello)
add_subdirectory(list)
