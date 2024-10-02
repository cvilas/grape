# =================================================================================================
# Copyright (C) 2023 GRAPE Contributors
# =================================================================================================

define_module_executable(
  NAME router
  SOURCES router.cpp
  PUBLIC_INCLUDE_PATHS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
  PUBLIC_LINK_LIBS grape::conio)
