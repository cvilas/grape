# =================================================================================================
# Copyright (C) 2026 GRAPE Contributors
# =================================================================================================

declare_module(
  NAME splat
  DEPENDS_ON_MODULES "base;conio"
  DEPENDS_ON_EXTERNAL_PROJECTS "SDL3")

find_package(SDL3 ${SDL3_VERSION_REQUIRED} REQUIRED)

# Shaders are written in GLSL and compiled to SPIR-V (for the SDL GPU Vulkan backend) at build time.
# The binaries are embedded into the library as generated headers.
find_program(GLSLANG_VALIDATOR glslangValidator REQUIRED)
set(SHADER_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/shaders)
set(SHADER_HEADERS "")
foreach(shader splat.vert splat.frag)
  string(REPLACE "." "_" shader_id ${shader})
  string(TOUPPER "${shader_id}_SPV" shader_var)
  set(shader_header ${SHADER_OUTPUT_DIR}/${shader_id}_spv.h)
  add_custom_command(
    OUTPUT ${shader_header}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${SHADER_OUTPUT_DIR}
    COMMAND ${GLSLANG_VALIDATOR} -V --target-env vulkan1.0 --quiet --vn ${shader_var} -o
            ${shader_header} ${CMAKE_CURRENT_SOURCE_DIR}/src/shaders/${shader}
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/shaders/${shader}
    COMMENT "Compiling shader ${shader}"
    VERBATIM)
  list(APPEND SHADER_HEADERS ${shader_header})
endforeach()
add_custom_target(${CMAKE_PROJECT_NAME}_splat_shaders DEPENDS ${SHADER_HEADERS})

set(HEADERS
    include/grape/splat/depth_sort.h include/grape/splat/examiner_camera.h
    include/grape/splat/file_io.h include/grape/splat/math.h include/grape/splat/splat.h
    include/grape/splat/viewer.h)
set(SOURCES
    src/async_sorter.h
    src/async_sorter.cpp
    src/depth_sort.cpp
    src/examiner_camera.cpp
    src/file_io.cpp
    src/gpu.h
    src/gpu.cpp
    src/splat.cpp
    src/viewer.cpp)

define_module_library(
  NAME splat
  SOURCES ${SOURCES}
  PUBLIC_HEADERS ${HEADERS}
  SYSTEM_PRIVATE_INCLUDE_PATHS ${SHADER_OUTPUT_DIR}
  PUBLIC_LINK_LIBS "grape::base;SDL3::SDL3")
add_dependencies(${CMAKE_PROJECT_NAME}_splat ${CMAKE_PROJECT_NAME}_splat_shaders)

add_subdirectory(apps)
add_subdirectory(examples)
add_subdirectory(tests)
