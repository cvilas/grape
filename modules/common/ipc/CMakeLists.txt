# =================================================================================================
# Copyright (C) 2023 GRAPE Contributors
# =================================================================================================

declare_module(
  NAME ipc
  DEPENDS_ON_MODULES "base"
  DEPENDS_ON_EXTERNAL_PROJECTS "zenohc;zenohcxx")

find_package(zenohc ${ZENOHC_VERSION_REQUIRED} REQUIRED)
find_package(zenohcxx ${ZENOHCXX_VERSION_REQUIRED} REQUIRED)

# library sources
set(SOURCES 
  src/ipc_zenoh.h
  src/session.cpp
  src/publisher.cpp
  src/subscriber.cpp
  include/grape/ipc/common.h
  include/grape/ipc/session.h
  include/grape/ipc/topic.h
  include/grape/ipc/publisher.h
  include/grape/ipc/subscriber.h 
  README.md)

# library target
define_module_library(
  NAME ipc
  PUBLIC_LINK_LIBS grape::base
  PRIVATE_LINK_LIBS ""
  SOURCES ${SOURCES}
  PUBLIC_INCLUDE_PATHS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                       $<INSTALL_INTERFACE:include>
  PRIVATE_INCLUDE_PATHS ""
  SYSTEM_PRIVATE_INCLUDE_PATHS ""
  PUBLIC_LINK_LIBS zenohc::lib)

# Subprojects
add_subdirectory(examples)
add_subdirectory(tests)
