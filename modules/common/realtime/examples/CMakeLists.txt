#=================================================================================================
# Copyright (C) 2023 GRAPE Contributors
#=================================================================================================

define_module_example(
        NAME mpscq_example
        SOURCES mpscq_example.cpp
        PUBLIC_INCLUDE_PATHS
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
        PUBLIC_LINK_LIBS "")

define_module_example(
        NAME mpscq_bench
        SOURCES mpscq_bench.cpp
        PUBLIC_INCLUDE_PATHS
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
        PRIVATE_LINK_LIBS benchmark::benchmark
        PUBLIC_LINK_LIBS "")
        
define_module_example(
        NAME fifo_example
        SOURCES fifo_example.cpp
        PUBLIC_INCLUDE_PATHS
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
        PUBLIC_LINK_LIBS "")

define_module_example(
        NAME fifo_bench
        SOURCES fifo_bench.cpp
        PUBLIC_INCLUDE_PATHS
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
        PRIVATE_LINK_LIBS benchmark::benchmark
        PUBLIC_LINK_LIBS "")
        
define_module_example(
        NAME string_example
        SOURCES string_example.cpp
        PUBLIC_INCLUDE_PATHS
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
        PUBLIC_LINK_LIBS "")

define_module_example(
        NAME schedule_example
        SOURCES schedule_example.cpp
        PUBLIC_INCLUDE_PATHS
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
        PUBLIC_LINK_LIBS "")
        
define_module_example(
        NAME thread_example
        SOURCES thread_example.cpp
        PUBLIC_INCLUDE_PATHS
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
        PUBLIC_LINK_LIBS "")
        