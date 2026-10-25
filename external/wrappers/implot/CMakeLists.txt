# =================================================================================================
# Copyright (C) 2026 GRAPE Contributors
# =================================================================================================

# Builds ImPlot from an upstream archive against the ImGui package built by ../imgui.
# Inputs: IMPLOT_SOURCE_DIR, IMPLOT_PACKAGE_VERSION, IMGUI_VERSION_REQUIRED, imgui_DIR
cmake_minimum_required(VERSION 4.3.0)
project(
  implot
  VERSION ${IMPLOT_PACKAGE_VERSION}
  LANGUAGES CXX)

include(${CMAKE_CURRENT_LIST_DIR}/../wrapper_package.cmake)

if(NOT EXISTS ${IMPLOT_SOURCE_DIR}/implot.cpp)
  message(FATAL_ERROR "IMPLOT_SOURCE_DIR must point to an extracted ImPlot archive")
endif()

find_package(imgui ${IMGUI_VERSION_REQUIRED} EXACT REQUIRED CONFIG)

add_library(implot)
add_library(implot::implot ALIAS implot)
target_compile_features(implot PUBLIC cxx_std_11)
target_link_libraries(implot PUBLIC imgui::imgui)
target_sources(
  implot
  PRIVATE ${IMPLOT_SOURCE_DIR}/implot.cpp
          ${IMPLOT_SOURCE_DIR}/implot_demo.cpp
          ${IMPLOT_SOURCE_DIR}/implot_items.cpp
  PUBLIC FILE_SET HEADERS
         BASE_DIRS ${IMPLOT_SOURCE_DIR}
         FILES ${IMPLOT_SOURCE_DIR}/implot.h
               ${IMPLOT_SOURCE_DIR}/implot_internal.h)

install_wrapper_package(TARGETS implot LICENSE ${IMPLOT_SOURCE_DIR}/LICENSE)
