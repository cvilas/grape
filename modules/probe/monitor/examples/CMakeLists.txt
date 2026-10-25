# =================================================================================================
# Copyright (C) 2024 GRAPE Contributors
# =================================================================================================

define_module_example(NAME example SOURCES example.cpp)

add_executable(imgui_demo EXCLUDE_FROM_ALL imgui_demo.cpp)
target_link_libraries(imgui_demo PRIVATE imgui::sdl3 imgui::sdlrenderer3)
set_target_properties(imgui_demo PROPERTIES CXX_CLANG_TIDY "" COMPILE_OPTIONS
                                                              "${THIRD_PARTY_COMPILER_WARNINGS}")

add_executable(implot_demo EXCLUDE_FROM_ALL implot_demo_main.cpp)
target_link_libraries(implot_demo PRIVATE implot::implot imgui::sdl3 imgui::sdlrenderer3)
set_target_properties(implot_demo PROPERTIES CXX_CLANG_TIDY "" COMPILE_OPTIONS
                                                               "${THIRD_PARTY_COMPILER_WARNINGS}")
apply_clang_format(implot_demo)

add_executable(implot_example EXCLUDE_FROM_ALL implot_example.cpp)
target_link_libraries(implot_example PRIVATE implot::implot imgui::sdl3 imgui::sdlrenderer3
                                             grape::utils)
set_target_properties(implot_example PROPERTIES CXX_CLANG_TIDY "")
apply_clang_format(implot_example)

add_dependencies(examples imgui_demo implot_demo implot_example)
