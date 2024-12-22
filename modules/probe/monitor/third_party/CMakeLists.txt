# =================================================================================================
# Copyright (C) 2024 GRAPE Contributors
# =================================================================================================

find_package(OpenGL REQUIRED)

# -------------------------------------------------------------------------------------------------
# Integrate imgui sources
set(IMGUI_SOURCES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/imgui)
set(IMGUI_SOURCES
    ${IMGUI_SOURCES_DIR}/imconfig.h
    ${IMGUI_SOURCES_DIR}/imgui.h
    ${IMGUI_SOURCES_DIR}/imgui_internal.h
    ${IMGUI_SOURCES_DIR}/imstb_rectpack.h
    ${IMGUI_SOURCES_DIR}/imstb_textedit.h
    ${IMGUI_SOURCES_DIR}/imstb_truetype.h
    ${IMGUI_SOURCES_DIR}/imgui.cpp
    ${IMGUI_SOURCES_DIR}/imgui_demo.cpp
    ${IMGUI_SOURCES_DIR}/imgui_draw.cpp
    ${IMGUI_SOURCES_DIR}/imgui_tables.cpp
    ${IMGUI_SOURCES_DIR}/imgui_widgets.cpp
    ${IMGUI_SOURCES_DIR}/backends/imgui_impl_glfw.h
    ${IMGUI_SOURCES_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_SOURCES_DIR}/backends/imgui_impl_opengl3.h
    ${IMGUI_SOURCES_DIR}/backends/imgui_impl_opengl3_loader.h
    ${IMGUI_SOURCES_DIR}/backends/imgui_impl_opengl3.cpp
    ${IMGUI_SOURCES_DIR}/misc/cpp/imgui_stdlib.h
    ${IMGUI_SOURCES_DIR}/misc/cpp/imgui_stdlib.cpp)

add_library(imguilib OBJECT ${IMGUI_SOURCES})
target_include_directories(imguilib PRIVATE ${IMGUI_SOURCES_DIR})
target_link_libraries(imguilib PUBLIC glfw OpenGL::GL)
set_target_properties(imguilib PROPERTIES CXX_CLANG_TIDY "")
set_target_properties(imguilib PROPERTIES COMPILE_OPTIONS ${THIRD_PARTY_COMPILER_WARNINGS})
set_target_properties(imguilib PROPERTIES POSITION_INDEPENDENT_CODE ON) # force -fPIC

add_executable(imgui_demo EXCLUDE_FROM_ALL ${IMGUI_SOURCES_DIR}/examples/demo.cpp)
add_dependencies(examples imgui_demo)
target_include_directories(imgui_demo PRIVATE ${IMGUI_SOURCES_DIR})
target_link_libraries(imgui_demo PRIVATE imguilib)
set_target_properties(imgui_demo PROPERTIES COMPILE_OPTIONS ${THIRD_PARTY_COMPILER_WARNINGS})
set_target_properties(imgui_demo PROPERTIES CXX_CLANG_TIDY "")

# -------------------------------------------------------------------------------------------------
# Integrate implot sources
set(IMPLOT_SOURCES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/implot)
set(IMPLOT_SOURCES
    ${IMPLOT_SOURCES_DIR}/implot.h ${IMPLOT_SOURCES_DIR}/implot_internal.h
    ${IMPLOT_SOURCES_DIR}/implot.cpp ${IMPLOT_SOURCES_DIR}/implot_items.cpp
    ${IMPLOT_SOURCES_DIR}/implot_demo.cpp)

add_library(implotlib OBJECT ${IMPLOT_SOURCES})
target_include_directories(implotlib PRIVATE ${IMGUI_SOURCES_DIR} ${IMPLOT_SOURCES_DIR})
target_link_libraries(implotlib PRIVATE imguilib)
set_target_properties(implotlib PROPERTIES CXX_CLANG_TIDY "")
set_target_properties(implotlib PROPERTIES COMPILE_OPTIONS ${THIRD_PARTY_COMPILER_WARNINGS})
set_target_properties(implotlib PROPERTIES POSITION_INDEPENDENT_CODE ON) # force -fPIC

add_executable(implot_demo EXCLUDE_FROM_ALL ${IMPLOT_SOURCES_DIR}/examples/implot_demo_main.cpp)
add_clang_format(implot_demo)
add_dependencies(examples implot_demo)
target_include_directories(implot_demo PRIVATE ${IMPLOT_SOURCES_DIR} ${IMGUI_SOURCES_DIR})
target_link_libraries(implot_demo PRIVATE implotlib imguilib)
set_target_properties(implot_demo PROPERTIES COMPILE_OPTIONS ${THIRD_PARTY_COMPILER_WARNINGS})
set_target_properties(implot_demo PROPERTIES CXX_CLANG_TIDY "")

add_executable(implot_example EXCLUDE_FROM_ALL ${IMPLOT_SOURCES_DIR}/examples/implot_example.cpp)
add_clang_format(implot_example)
add_dependencies(examples implot_example)
target_include_directories(implot_example SYSTEM PRIVATE ${IMPLOT_SOURCES_DIR} ${IMGUI_SOURCES_DIR})
target_link_libraries(implot_example PRIVATE implotlib imguilib)
