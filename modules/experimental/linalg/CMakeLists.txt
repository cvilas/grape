# =================================================================================================
# Copyright (C) 2025 GRAPE Contributors
# =================================================================================================

declare_module(
  NAME linalg
  DEPENDS_ON_MODULES "utils"
  DEPENDS_ON_EXTERNAL_PROJECTS "")

# Unity builds compile a generated source in the build tree, which clang-tidy would otherwise lint
# against the top-level config. Mirror this module's lint config next to the generated sources.
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/.clang-tidy ${CMAKE_CURRENT_BINARY_DIR}/.clang-tidy
               COPYONLY)

# library sources
set(SOURCES)
set(PUBLIC_HEADERS include/grape/linalg/frames.h include/grape/linalg/matrix.h
                   include/grape/linalg/quaternion.h)

# library target
define_module_library(
  NAME linalg
  PUBLIC_LINK_LIBS grape::utils
  PRIVATE_LINK_LIBS ""
  SOURCES ${SOURCES}
  PUBLIC_HEADERS ${PUBLIC_HEADERS}
  PRIVATE_INCLUDE_PATHS ""
  SYSTEM_PRIVATE_INCLUDE_PATHS "")

# Subprojects
add_subdirectory(tests)
add_subdirectory(examples)
