# =================================================================================================
# Copyright (C) 2023 GRAPE Contributors
# =================================================================================================

declare_module(
  NAME log
  DEPENDS_ON_MODULES "realtime;utils"
  DEPENDS_ON_EXTERNAL_PROJECTS "")

# library sources
set(SOURCES
    README.md
    include/grape/log/config.h
    include/grape/log/default_formatter.h
    include/grape/log/default_sink.h
    include/grape/log/logger.h
    include/grape/log/record.h
    include/grape/log/severity.h
    src/logger.cpp)

# library target
define_module_library(
  NAME log
  PUBLIC_LINK_LIBS grape::utils grape::realtime
  PRIVATE_LINK_LIBS ""
  SOURCES ${SOURCES}
  PUBLIC_INCLUDE_PATHS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                       $<INSTALL_INTERFACE:include>
  PRIVATE_INCLUDE_PATHS ""
  SYSTEM_PRIVATE_INCLUDE_PATHS "")

# Subprojects
add_subdirectory(tests)
add_subdirectory(examples)
