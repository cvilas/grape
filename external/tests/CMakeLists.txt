# =================================================================================================
# Copyright (C) 2026 GRAPE Contributors
# =================================================================================================

cmake_minimum_required(VERSION 4.3.0)
project(grape-gui-package-tests LANGUAGES CXX)

include(CTest)
include(${CMAKE_CURRENT_LIST_DIR}/../third_party_versions.cmake)
find_package(implot ${IMPLOT_VERSION_REQUIRED} EXACT REQUIRED CONFIG)
if(GUI_EXPECTED_LIBRARY_TYPE)
  foreach(TARGET imgui::imgui imgui::sdl3 imgui::sdlrenderer3 imgui::sdlgpu3 implot::implot)
    get_target_property(LIBRARY_TYPE ${TARGET} TYPE)
    if(NOT LIBRARY_TYPE STREQUAL GUI_EXPECTED_LIBRARY_TYPE)
      message(FATAL_ERROR "${TARGET}: expected ${GUI_EXPECTED_LIBRARY_TYPE}, got ${LIBRARY_TYPE}")
    endif()
  endforeach()
endif()

add_executable(renderer_smoke renderer_smoke.cpp)
target_link_libraries(renderer_smoke PRIVATE implot::implot imgui::sdl3 imgui::sdlrenderer3)
target_compile_features(renderer_smoke PRIVATE cxx_std_17)
target_compile_definitions(renderer_smoke PRIVATE TEST_DEMOS)
add_test(NAME renderer_smoke COMMAND renderer_smoke)
set_tests_properties(renderer_smoke PROPERTIES ENVIRONMENT "SDL_VIDEODRIVER=dummy")

add_executable(gpu_link gpu_link.cpp)
target_link_libraries(gpu_link PRIVATE implot::implot imgui::sdl3 imgui::sdlgpu3)
add_test(NAME gpu_link COMMAND gpu_link)
