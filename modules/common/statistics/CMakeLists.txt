# =================================================================================================
# Copyright (C) 2025 GRAPE Contributors
# =================================================================================================

declare_module(
  NAME statistics
  DEPENDS_ON_MODULES ""
  DEPENDS_ON_EXTERNAL_PROJECTS "")

# library sources
set(SOURCES README.md include/grape/statistics/sliding_mean.h)

# library target
define_module_library(
  NAME statistics
  PUBLIC_LINK_LIBS ""
  PRIVATE_LINK_LIBS ""
  SOURCES ${SOURCES}
  PUBLIC_INCLUDE_PATHS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                       $<INSTALL_INTERFACE:include>
  PRIVATE_INCLUDE_PATHS ""
  SYSTEM_PRIVATE_INCLUDE_PATHS "")

# Subprojects
add_subdirectory(tests)
add_subdirectory(examples)
