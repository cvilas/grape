# =================================================================================================
# Copyright (C) 2023 GRAPE Contributors
# =================================================================================================

define_module_example(NAME example SOURCES example.cpp)

define_module_example(NAME custom_sink_example SOURCES custom_sink_example.cpp)

define_module_bench(
  NAME spdlog_bench
  SOURCES spdlog_bench.cpp
  SYSTEM_PRIVATE_INCLUDE_PATHS ${CMAKE_CURRENT_SOURCE_DIR}/spdlog-1.17.0/include
  PRIVATE_INCLUDE_PATHS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
  PUBLIC_LINK_LIBS "")
set_target_properties(grape_log_spdlog_bench PROPERTIES CXX_CLANG_TIDY "") # disable spdlog warnings
target_compile_options(grape_log_spdlog_bench PRIVATE ${THIRD_PARTY_COMPILER_WARNINGS})
