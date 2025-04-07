# =================================================================================================
# Copyright (C) 2023 GRAPE Contributors
# =================================================================================================

declare_module(
  NAME log
  DEPENDS_ON_MODULES "realtime;utils;base"
  DEPENDS_ON_EXTERNAL_PROJECTS "")

# library sources
set(SOURCES
    README.md
    include/grape/log/formatters/default_formatter.h
    include/grape/log/sinks/sink.h
    include/grape/log/sinks/console_sink.h
    include/grape/log/sinks/file_sink.h
    include/grape/log/config.h
    include/grape/log/logger.h
    include/grape/log/record.h
    include/grape/log/severity.h
    include/grape/log/syslog.h
    src/logger.cpp
    src/syslog.cpp)

# library target
define_module_library(
  NAME log
  PUBLIC_LINK_LIBS grape::base grape::utils grape::realtime
  PRIVATE_LINK_LIBS ""
  SOURCES ${SOURCES}
  PUBLIC_INCLUDE_PATHS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                       $<INSTALL_INTERFACE:include>
  PRIVATE_INCLUDE_PATHS ""
  SYSTEM_PRIVATE_INCLUDE_PATHS "")

# Subprojects
add_subdirectory(tests)
add_subdirectory(examples)
