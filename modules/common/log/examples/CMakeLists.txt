# =================================================================================================
# Copyright (C) 2023 GRAPE Contributors
# =================================================================================================

define_module_example(
  NAME example
  SOURCES example.cpp
  PUBLIC_INCLUDE_PATHS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
  PUBLIC_LINK_LIBS "")

define_module_example(
  NAME custom_sink_example
  SOURCES custom_sink_example.cpp
  PUBLIC_INCLUDE_PATHS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
  PUBLIC_LINK_LIBS "")

define_module_example(
  NAME spdlog_bench
  SOURCES spdlog_bench.cpp
  PRIVATE_INCLUDE_PATHS ${CMAKE_CURRENT_SOURCE_DIR}/spdlog-1.15.0/include
                        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
  PRIVATE_LINK_LIBS benchmark::benchmark
  PUBLIC_LINK_LIBS "")
set_target_properties(grape_log_spdlog_bench PROPERTIES CXX_CLANG_TIDY "") # disable spdlog warnings
set_target_properties(grape_log_spdlog_bench PROPERTIES COMPILE_OPTIONS
                                                        "${THIRD_PARTY_COMPILER_WARNINGS}")
