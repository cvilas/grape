# =================================================================================================
# Copyright (C) 2024 GRAPE Contributors
# =================================================================================================

define_module_example(
  NAME example
  SOURCES example.cpp
  PUBLIC_INCLUDE_PATHS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
  PUBLIC_LINK_LIBS "")

define_module_example(
  NAME advanced_example
  SOURCES advanced_example.cpp 
  PUBLIC_INCLUDE_PATHS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
  PUBLIC_LINK_LIBS "")

define_module_example(
  NAME bench
  SOURCES bench.cpp
  PRIVATE_INCLUDE_PATHS ""
  PRIVATE_LINK_LIBS benchmark::benchmark
  PUBLIC_LINK_LIBS "")
