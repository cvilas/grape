# =================================================================================================
# Copyright (C) @year@ @project@ Contributors
# =================================================================================================

declare_module(
  NAME @module@
  DEPENDS_ON_MODULES ""
  DEPENDS_ON_EXTERNAL_PROJECTS "")

# library sources
set(HEADERS include/@project@/@module@/@module@.h)
set(SOURCES src/@module@.cpp)

# library target
define_module_library(
  NAME @module@
  PUBLIC_LINK_LIBS ""
  PRIVATE_LINK_LIBS ""
  SOURCES ${SOURCES}
  PUBLIC_HEADERS ${HEADERS}
  PRIVATE_INCLUDE_PATHS ""
  SYSTEM_PRIVATE_INCLUDE_PATHS "")

# Subprojects
add_subdirectory(tests)
add_subdirectory(examples)
