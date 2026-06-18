# =================================================================================================
# Copyright (C) 2024 GRAPE Contributors
# =================================================================================================

declare_module(
  NAME probe_monitor
  DEPENDS_ON_MODULES "probe_controller"
  DEPENDS_ON_EXTERNAL_PROJECTS "SDL3")

find_package(SDL3 ${SDL3_VERSION_REQUIRED} REQUIRED)

# library sources
set(HEADERS include/grape/probe/monitor.h)
set(SOURCES src/monitor.cpp)

# library target
define_module_library(
  NAME probe_monitor
  PUBLIC_LINK_LIBS "grape::probe_controller"
  PRIVATE_LINK_LIBS $<BUILD_LOCAL_INTERFACE:imguilib> $<BUILD_LOCAL_INTERFACE:implotlib>
  SOURCES ${SOURCES}
  PUBLIC_HEADERS ${HEADERS}
  PRIVATE_INCLUDE_PATHS ""
  SYSTEM_PRIVATE_INCLUDE_PATHS
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/imgui/ ${CMAKE_CURRENT_SOURCE_DIR}/third_party/implot/
    ${CMAKE_BINARY_DIR}/external/deploy/include)

# Subprojects
add_subdirectory(third_party)
add_subdirectory(examples)
