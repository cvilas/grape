# =================================================================================================
# Copyright (C) 2026 GRAPE Contributors
# =================================================================================================

# Builds Dear ImGui with SDL3 platform, SDL_Renderer and SDL_GPU backends from an upstream archive.
# Inputs: IMGUI_SOURCE_DIR, IMGUI_PACKAGE_VERSION, SDL3_VERSION_REQUIRED
cmake_minimum_required(VERSION 4.3.0)
project(
  imgui
  VERSION ${IMGUI_PACKAGE_VERSION}
  LANGUAGES CXX)

include(${CMAKE_CURRENT_LIST_DIR}/../wrapper_package.cmake)

if(NOT EXISTS ${IMGUI_SOURCE_DIR}/imgui.cpp)
  message(FATAL_ERROR "IMGUI_SOURCE_DIR must point to an extracted Dear ImGui archive")
endif()

find_package(SDL3 ${SDL3_VERSION_REQUIRED} REQUIRED CONFIG)

add_library(imgui)
add_library(imgui::imgui ALIAS imgui)
target_compile_features(imgui PUBLIC cxx_std_11)
target_sources(
  imgui
  PRIVATE ${IMGUI_SOURCE_DIR}/imgui.cpp
          ${IMGUI_SOURCE_DIR}/imgui_demo.cpp
          ${IMGUI_SOURCE_DIR}/imgui_draw.cpp
          ${IMGUI_SOURCE_DIR}/imgui_tables.cpp
          ${IMGUI_SOURCE_DIR}/imgui_widgets.cpp
          ${IMGUI_SOURCE_DIR}/misc/cpp/imgui_stdlib.cpp
  PUBLIC FILE_SET HEADERS
         BASE_DIRS ${IMGUI_SOURCE_DIR}
         FILES ${IMGUI_SOURCE_DIR}/imconfig.h
               ${IMGUI_SOURCE_DIR}/imgui.h
               ${IMGUI_SOURCE_DIR}/imgui_internal.h
               ${IMGUI_SOURCE_DIR}/imstb_rectpack.h
               ${IMGUI_SOURCE_DIR}/imstb_textedit.h
               ${IMGUI_SOURCE_DIR}/imstb_truetype.h
               ${IMGUI_SOURCE_DIR}/misc/cpp/imgui_stdlib.h)

# Each backend is a separate library (imgui::<backend>) so applications link only what they use
set(IMGUI_BACKENDS sdl3 sdlrenderer3 sdlgpu3)
foreach(backend IN LISTS IMGUI_BACKENDS)
  set(target imgui_${backend})
  add_library(${target})
  add_library(imgui::${backend} ALIAS ${target})
  target_sources(
    ${target}
    PRIVATE ${IMGUI_SOURCE_DIR}/backends/imgui_impl_${backend}.cpp
    PUBLIC FILE_SET HEADERS
           BASE_DIRS ${IMGUI_SOURCE_DIR}
           FILES ${IMGUI_SOURCE_DIR}/backends/imgui_impl_${backend}.h)
  target_link_libraries(${target} PUBLIC imgui SDL3::SDL3)
  set_target_properties(${target} PROPERTIES EXPORT_NAME ${backend})
endforeach()
list(TRANSFORM IMGUI_BACKENDS PREPEND imgui_ OUTPUT_VARIABLE IMGUI_BACKEND_TARGETS)

install_wrapper_package(TARGETS imgui ${IMGUI_BACKEND_TARGETS} LICENSE ${IMGUI_SOURCE_DIR}/LICENSE.txt)
