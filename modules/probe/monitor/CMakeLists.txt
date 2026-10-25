# =================================================================================================
# Copyright (C) 2024 GRAPE Contributors
# =================================================================================================

declare_module(
  NAME probe_monitor
  DEPENDS_ON_MODULES "probe_controller"
  DEPENDS_ON_EXTERNAL_PROJECTS "SDL3;imgui;implot")

find_package(SDL3 ${SDL3_VERSION_REQUIRED} REQUIRED)
find_package(imgui ${IMGUI_VERSION_REQUIRED} EXACT REQUIRED CONFIG)
find_package(implot ${IMPLOT_VERSION_REQUIRED} EXACT REQUIRED CONFIG)

# library sources
set(HEADERS include/grape/probe/monitor.h)
set(SOURCES src/monitor.cpp)

# library target
define_module_library(
  NAME probe_monitor
  PUBLIC_LINK_LIBS "grape::probe_controller"
  PRIVATE_LINK_LIBS "implot::implot;imgui::sdl3;imgui::sdlrenderer3"
  SOURCES ${SOURCES}
  PUBLIC_HEADERS ${HEADERS})

# Subprojects
add_subdirectory(examples)
