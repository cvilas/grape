# =================================================================================================
# Copyright (C) 2025 GRAPE Contributors
# =================================================================================================

declare_module(
  NAME linalg
  DEPENDS_ON_MODULES ""
  DEPENDS_ON_EXTERNAL_PROJECTS "")

# library sources
set(SOURCES)
set(PUBLIC_HEADERS include/grape/linalg/matrix.h)

# library target
define_module_library(
  NAME linalg
  PUBLIC_LINK_LIBS ""
  PRIVATE_LINK_LIBS ""
  SOURCES ${SOURCES}
  PUBLIC_HEADERS ${PUBLIC_HEADERS}
  PRIVATE_INCLUDE_PATHS ""
  SYSTEM_PRIVATE_INCLUDE_PATHS "")

# Subprojects
add_subdirectory(tests)
add_subdirectory(examples)
