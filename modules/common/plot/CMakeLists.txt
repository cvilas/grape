# =================================================================================================
# Copyright (C) 2026 GRAPE Contributors
# =================================================================================================

declare_module(
  NAME plot
  DEPENDS_ON_MODULES "base"
  DEPENDS_ON_EXTERNAL_PROJECTS "SDL3;SDL3_ttf")

find_package(SDL3 ${SDL3_VERSION_REQUIRED} REQUIRED)
find_package(SDL3_ttf ${SDL3_TTF_VERSION_REQUIRED} REQUIRED)

set(SOURCES
    src/window.cpp
    src/trace.cpp
    src/font.cpp
    src/font.h
    include/grape/plot/snapshot_buffer.h
    include/grape/plot/style.h
    include/grape/plot/window.h
    include/grape/plot/trace.h)

define_module_library(
  NAME plot
  SOURCES ${SOURCES}
  PUBLIC_INCLUDE_PATHS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                       $<INSTALL_INTERFACE:include>
  PUBLIC_LINK_LIBS "grape::base;SDL3::SDL3;SDL3_ttf::SDL3_ttf")

add_subdirectory(examples)
add_subdirectory(tests)
