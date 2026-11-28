# =================================================================================================
# Copyright (C) 2025 GRAPE Contributors
# =================================================================================================

define_module_example(
  NAME example
  SOURCES example.cpp
  PUBLIC_INCLUDE_PATHS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
  PUBLIC_LINK_LIBS "")

# Performance benchmarks (see ../docs/benchmarking.md). When Eigen is available, each linalg
# benchmark is paired with an equivalent Eigen benchmark in the same executable for direct
# comparison; otherwise only the linalg benchmarks are built.
define_module_bench(
  NAME bench
  SOURCES bench.cpp
  PUBLIC_INCLUDE_PATHS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
  PUBLIC_LINK_LIBS "")
set(LINALG_BENCH_TARGET ${CMAKE_PROJECT_NAME}_${MODULE_NAME}_bench)
target_compile_definitions(${LINALG_BENCH_TARGET} PRIVATE GRAPE_TARGET_ARCH="${TARGET_ARCH}")

find_package(Eigen3 QUIET NO_MODULE)
if(Eigen3_FOUND)
  target_compile_definitions(${LINALG_BENCH_TARGET} PRIVATE GRAPE_LINALG_BENCH_EIGEN)
  target_link_libraries(${LINALG_BENCH_TARGET} PRIVATE Eigen3::Eigen)
  if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    # -Wnull-dereference false-positives in code inlined from Eigen >= 5 (PartialPivLU)
    target_compile_options(${LINALG_BENCH_TARGET} PRIVATE -Wno-null-dereference)
  endif()
  message(STATUS "linalg benchmarks: comparing against Eigen ${Eigen3_VERSION}")
else()
  message(STATUS "linalg benchmarks: Eigen not found, building linalg benchmarks only")
endif()
