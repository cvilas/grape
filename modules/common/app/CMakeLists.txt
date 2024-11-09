# =================================================================================================
# Copyright (C) 2024 GRAPE Contributors
# =================================================================================================

declare_module(
  NAME app
  DEPENDS_ON_MODULES "conio;log;script;serdes;ipc"
  DEPENDS_ON_EXTERNAL_PROJECTS "")

# library sources
set(SOURCES 
  src/detail/application.cpp 
  src/app.cpp 
  README.md 
  include/grape/app/app.h
  include/grape/app/detail/application.h)

# library target
define_module_library(
  NAME app
  PUBLIC_LINK_LIBS grape::conio grape::log grape::script grape::serdes grape::ipc
  PRIVATE_LINK_LIBS ""
  SOURCES ${SOURCES}
  PUBLIC_INCLUDE_PATHS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                       $<INSTALL_INTERFACE:include>
  PRIVATE_INCLUDE_PATHS ""
  SYSTEM_PRIVATE_INCLUDE_PATHS "")

# Subprojects
add_subdirectory(config)
add_subdirectory(tests)
add_subdirectory(examples)
