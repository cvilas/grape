# =================================================================================================
# Copyright (C) 2025 GRAPE Contributors
# =================================================================================================

define_module_test(
  NAME tests
  SOURCES sliding_mean_tests.cpp
  PUBLIC_INCLUDE_PATHS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
  PUBLIC_LINK_LIBS "")
