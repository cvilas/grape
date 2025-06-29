# =================================================================================================
# Copyright (C) 2025 GRAPE Contributors
# =================================================================================================

define_module_example(
  NAME sliding_mean_example
  SOURCES sliding_mean_example.cpp
  PUBLIC_INCLUDE_PATHS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
  PUBLIC_LINK_LIBS "")
