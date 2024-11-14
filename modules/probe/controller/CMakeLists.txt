# =================================================================================================
# Copyright (C) 2023 GRAPE Contributors
# =================================================================================================

declare_module(
  NAME probe_controller
  DEPENDS_ON_MODULES "realtime;utils"
  DEPENDS_ON_EXTERNAL_PROJECTS "")

# library sources
set(SOURCES src/controller.cpp README.md include/grape/probe/controller.h
            include/grape/probe/signal.h include/grape/probe/type_id.h)

# library target
define_module_library(
  NAME probe_controller
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
