# =================================================================================================
# Copyright (C) 2025 GRAPE Contributors
# =================================================================================================

declare_module(
  NAME camera
  DEPENDS_ON_MODULES "statistics;ipc;log;conio;script"
  DEPENDS_ON_EXTERNAL_PROJECTS "SDL3;lz4")

find_package(SDL3 ${SDL3_VERSION_REQUIRED} REQUIRED)
find_package(lz4 ${LZ4_VERSION_REQUIRED} REQUIRED)

set(SOURCES
    include/grape/camera/camera.h
    include/grape/camera/compressor.h
    include/grape/camera/decompressor.h
    include/grape/camera/display.h
    include/grape/camera/formatter.h
    include/grape/camera/image_frame.h
    include/grape/camera/rate_limiter.h
    include/grape/camera/scaler.h
    src/camera.cpp
    src/compressor.cpp
    src/decompressor.cpp
    src/display.cpp
    src/formatter.cpp
    src/image_frame.cpp
    src/rate_limiter.cpp
    src/scaler.cpp
    src/helpers.h
    src/helpers.cpp)

define_module_library(
  NAME camera
  SOURCES ${SOURCES}
  PUBLIC_INCLUDE_PATHS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                       $<INSTALL_INTERFACE:include>
  PUBLIC_LINK_LIBS "grape::statistics;grape::log;grape::ipc;SDL3::SDL3;LZ4::lz4")

add_subdirectory(apps)
