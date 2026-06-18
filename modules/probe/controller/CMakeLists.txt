# =================================================================================================
# Copyright (C) 2023 GRAPE Contributors
# =================================================================================================

declare_module(
  NAME probe_controller
  DEPENDS_ON_MODULES "realtime"
  DEPENDS_ON_EXTERNAL_PROJECTS "")

# library sources
set(HEADERS include/grape/probe/controller.h include/grape/probe/signal.h
            include/grape/probe/type_id.h)
set(SOURCES src/controller.cpp)

# library target
define_module_library(
  NAME probe_controller
  PUBLIC_LINK_LIBS "grape::realtime"
  PRIVATE_LINK_LIBS ""
  SOURCES ${SOURCES}
  PUBLIC_HEADERS ${HEADERS}
  PRIVATE_INCLUDE_PATHS ""
  SYSTEM_PRIVATE_INCLUDE_PATHS "")

# Subprojects
add_subdirectory(tests)
add_subdirectory(examples)
