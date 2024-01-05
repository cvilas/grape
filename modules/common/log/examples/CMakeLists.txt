#=================================================================================================
# Copyright (C) 2018-2023 GRAPE Contributors
#=================================================================================================

define_module_example(
        NAME example
        SOURCES example.cpp
        PUBLIC_INCLUDE_PATHS
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
        PUBLIC_LINK_LIBS "")

define_module_example(
        NAME custom_sink_example
        SOURCES custom_sink_example.cpp
        PUBLIC_INCLUDE_PATHS
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
        PUBLIC_LINK_LIBS "")

define_module_example(
        NAME spdlog_bench
        SOURCES spdlog_bench.cpp
        PRIVATE_INCLUDE_PATHS ${CMAKE_CURRENT_SOURCE_DIR}/spdlog/include
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
        PUBLIC_LINK_LIBS "")
set_target_properties(grape_log_spdlog_bench PROPERTIES CXX_CLANG_TIDY "") # turn off spdlog linter warnings
        