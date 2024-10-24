# =================================================================================================
# Copyright (C) 2024 GRAPE Contributors
# =================================================================================================

declare_module(
  NAME recorder
  DEPENDS_ON_MODULES ""
  DEPENDS_ON_EXTERNAL_PROJECTS mcap)

find_package(mcap ${MCAP_VERSION_REQUIRED} REQUIRED)

# library sources
set(SOURCES 
  include/grape/recorder/recorder.h
  README.md)

# library target
define_module_library(
  NAME recorder
  PUBLIC_LINK_LIBS mcap::mcap
  PRIVATE_LINK_LIBS ""
  SOURCES ${SOURCES}
  PUBLIC_INCLUDE_PATHS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                       $<INSTALL_INTERFACE:include>
  PRIVATE_INCLUDE_PATHS ""
  SYSTEM_PRIVATE_INCLUDE_PATHS "")

# Subprojects
#add_subdirectory(tests)
add_subdirectory(examples)
