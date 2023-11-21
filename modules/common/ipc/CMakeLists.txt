#=================================================================================================
# Copyright (C) 2018-2023 GRAPE Contributors
#=================================================================================================

declare_module(NAME ipc DEPENDS_ON_MODULES "" DEPENDS_ON_EXTERNAL_PROJECTS zenohcxx)

find_package(zenohc REQUIRED)
find_package(zenohcxx REQUIRED)

# library sources
set(SOURCES
        include/grape/ipc/ipc.h
        src/ipc.cpp
        README.md)

# library target
define_module_library(
        NAME ipc
        PUBLIC_LINK_LIBS ""
        PRIVATE_LINK_LIBS ""
        SOURCES ${SOURCES}
        PUBLIC_INCLUDE_PATHS
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:include>
        PRIVATE_INCLUDE_PATHS ""
        SYSTEM_PRIVATE_INCLUDE_PATHS ""
        PUBLIC_LINK_LIBS zenohc::lib)

# Subprojects
add_subdirectory(apps)
add_subdirectory(examples)
add_subdirectory(tests)
