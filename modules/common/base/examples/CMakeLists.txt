# =================================================================================================
# Copyright (C) 2023 GRAPE Contributors
# =================================================================================================

define_module_example(
  NAME exception_example
  SOURCES exception_example.cpp
  PUBLIC_INCLUDE_PATHS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  PUBLIC_LINK_LIBS "")
