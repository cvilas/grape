# =================================================================================================
# Copyright (C) 2025 GRAPE Contributors
# =================================================================================================

declare_module(
  NAME ipc
  DEPENDS_ON_MODULES "utils;conio"
  DEPENDS_ON_EXTERNAL_PROJECTS "eCAL")

find_package(eCAL REQUIRED) # TODO(vilas): ${ECAL_VERSION_REQUIRED} REQUIRED)

# library sources
set(SOURCES
    src/publisher_impl.h
    src/subscriber_impl.h
    src/publisher.cpp
    src/session.cpp
    src/subscriber.cpp
    README.md
    include/grape/ipc/match.h
    include/grape/ipc/publisher.h
    include/grape/ipc/session.h
    include/grape/ipc/subscriber.h
    include/grape/ipc/topic.h)

# library target
define_module_library(
  NAME ipc
  PUBLIC_LINK_LIBS grape::utils eCAL::core
  PRIVATE_LINK_LIBS ""
  SOURCES ${SOURCES}
  PUBLIC_INCLUDE_PATHS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                       $<INSTALL_INTERFACE:include>
  PRIVATE_INCLUDE_PATHS ""
  SYSTEM_PRIVATE_INCLUDE_PATHS "")

# Subprojects
add_subdirectory(tests)
add_subdirectory(examples)
