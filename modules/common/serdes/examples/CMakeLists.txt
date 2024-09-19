# =================================================================================================
# Copyright (C) 2024 GRAPE Contributors
# =================================================================================================

define_module_example(
  NAME cdr_example
  SOURCES cdr_example.cpp examples_utils.h
  PUBLIC_INCLUDE_PATHS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
  PUBLIC_LINK_LIBS "")

define_module_example(
  NAME msgpk_example
  SOURCES msgpk_example.cpp examples_utils.h
  PUBLIC_INCLUDE_PATHS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
  PUBLIC_LINK_LIBS "")

define_module_example(
  NAME flxb_example
  SOURCES flxb_example.cpp examples_utils.h
  PUBLIC_INCLUDE_PATHS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
  PUBLIC_LINK_LIBS "")

define_module_example(
  NAME bench
  SOURCES bench.cpp
  PRIVATE_INCLUDE_PATHS ""
  PRIVATE_LINK_LIBS benchmark::benchmark
  PUBLIC_LINK_LIBS "")
