# =================================================================================================
# Copyright (C) 2023 GRAPE Contributors
# =================================================================================================

declare_module(
  NAME utils
  ALWAYS_BUILD
  DEPENDS_ON_MODULES "" # **Note**: This module should not depend on any other module
  DEPENDS_ON_EXTERNAL_PROJECTS "")

# generate version header
configure_file(src/version.in ${CMAKE_CURRENT_SOURCE_DIR}/src/version_impl.h @ONLY)

# library sources
set(SOURCES
    src/ip.cpp
    src/stacktrace.cpp
    src/utils.cpp
    src/version.cpp
    README.md
    include/grape/utils/enums.h
    include/grape/utils/ip.h
    include/grape/utils/stacktrace.h
    include/grape/utils/utils.h
    include/grape/utils/version.h)

# library target
define_module_library(
  NAME utils
  PUBLIC_LINK_LIBS ""
  PRIVATE_LINK_LIBS ""
  SOURCES ${SOURCES}
  PUBLIC_INCLUDE_PATHS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                       $<INSTALL_INTERFACE:include>
  PRIVATE_INCLUDE_PATHS ""
  SYSTEM_PRIVATE_INCLUDE_PATHS "")

# Subprojects
add_subdirectory(examples)
add_subdirectory(tests)
add_subdirectory(apps)
