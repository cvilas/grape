# =================================================================================================
# Copyright (C) 2025 GRAPE Contributors
# =================================================================================================

declare_module(
  NAME ipc
  DEPENDS_ON_MODULES "base;serdes;conio"
  DEPENDS_ON_EXTERNAL_PROJECTS "eCAL")

find_package(eCAL ${ECAL_VERSION_REQUIRED} REQUIRED)

# library sources
set(HEADERS
    include/grape/ipc/discovery.h
    include/grape/ipc/entity_id.h
    include/grape/ipc/error.h
    include/grape/ipc/config.h
    include/grape/ipc/match.h
    include/grape/ipc/qos.h
    include/grape/ipc/session.h
    include/grape/ipc/raw_publisher.h
    include/grape/ipc/raw_subscriber.h
    include/grape/ipc/topic.h
    include/grape/ipc/publisher.h
    include/grape/ipc/subscriber.h)

set(SOURCES src/default_config.h src/discovery.cpp src/session.cpp src/raw_publisher.cpp
            src/raw_subscriber.cpp)

# library target
define_module_library(
  NAME ipc
  PUBLIC_LINK_LIBS "grape::base;grape::serdes;eCAL::core"
  PRIVATE_LINK_LIBS ""
  SOURCES ${SOURCES}
  PUBLIC_HEADERS ${HEADERS}
  PRIVATE_INCLUDE_PATHS ""
  SYSTEM_PRIVATE_INCLUDE_PATHS "")

# Subprojects
if(NOT CMAKE_CROSSCOMPILING)
  add_subdirectory(py)
endif()
add_subdirectory(tests)
add_subdirectory(examples)
add_subdirectory(apps)
