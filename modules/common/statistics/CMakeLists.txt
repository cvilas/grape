# =================================================================================================
# Copyright (C) 2025 GRAPE Contributors
# =================================================================================================

declare_module(
  NAME statistics
  DEPENDS_ON_MODULES ""
  DEPENDS_ON_EXTERNAL_PROJECTS "")

# library sources
set(HEADERS include/grape/statistics/sliding_mean.h)
set(SOURCES)

# library target
define_module_library(
  NAME statistics
  PUBLIC_LINK_LIBS ""
  PRIVATE_LINK_LIBS ""
  SOURCES ${SOURCES}
  PUBLIC_HEADERS ${HEADERS}
  PRIVATE_INCLUDE_PATHS ""
  SYSTEM_PRIVATE_INCLUDE_PATHS "")

# Subprojects
add_subdirectory(tests)
add_subdirectory(examples)
