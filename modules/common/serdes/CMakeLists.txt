# =================================================================================================
# Copyright (C) 2024 GRAPE Contributors
# =================================================================================================

declare_module(
  NAME serdes
  DEPENDS_ON_MODULES "base"
  DEPENDS_ON_EXTERNAL_PROJECTS "")

# library sources
set(HEADERS include/grape/serdes/concepts.h include/grape/serdes/serdes.h
            include/grape/serdes/stream.h)
set(SOURCES)

# library target
define_module_library(
  NAME serdes
  PUBLIC_LINK_LIBS "grape::base"
  PRIVATE_LINK_LIBS ""
  SOURCES ${SOURCES}
  PUBLIC_HEADERS ${HEADERS}
  PRIVATE_INCLUDE_PATHS ""
  SYSTEM_PRIVATE_INCLUDE_PATHS "")

# Subprojects
add_subdirectory(tests)
add_subdirectory(examples)
