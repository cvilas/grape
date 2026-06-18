# =================================================================================================
# Copyright (C) 2023 GRAPE Contributors
# =================================================================================================

# declare_module(NAME ipc2 DEPENDS_ON_MODULES "base" DEPENDS_ON_EXTERNAL_PROJECTS "zenohc;zenohcxx")

find_package(zenohc ${ZENOHC_VERSION_REQUIRED} REQUIRED)
find_package(zenohcxx ${ZENOHCXX_VERSION_REQUIRED} REQUIRED)

# library sources
set(HEADERS include/grape/ipc2/common.h include/grape/ipc2/session.h include/grape/ipc2/topic.h
            include/grape/ipc2/publisher.h include/grape/ipc2/subscriber.h)

set(SOURCES src/common.cpp src/ipc_zenoh.h src/session.cpp src/publisher.cpp src/subscriber.cpp)

# library target
define_module_library(
  NAME ipc2
  PUBLIC_LINK_LIBS grape::base
  PRIVATE_LINK_LIBS ""
  SOURCES ${SOURCES}
  PUBLIC_HEADERS ${HEADERS}
  PRIVATE_INCLUDE_PATHS ""
  SYSTEM_PRIVATE_INCLUDE_PATHS ""
  PUBLIC_LINK_LIBS zenohc::lib)

# Subprojects
add_subdirectory(examples)
add_subdirectory(tests)
