#=================================================================================================
# Copyright (C) 2018-2023 GRAPE Contributors
#=================================================================================================

declare_module(NAME utils ALWAYS_BUILD DEPENDS_ON_MODULES "" DEPENDS_ON_EXTERNAL_PROJECTS "")

# library sources
set(SOURCES
        src/utils.cpp
        src/command_line_args.cpp
        README.md
        include/grape/utils/utils.h
        include/grape/utils/command_line_args.h)

# library target
define_module_library(
        NAME utils
        PUBLIC_LINK_LIBS ""
        PRIVATE_LINK_LIBS ""
        SOURCES ${SOURCES}
        PUBLIC_INCLUDE_PATHS
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:include>
        PRIVATE_INCLUDE_PATHS ""
        SYSTEM_PRIVATE_INCLUDE_PATHS "")

# Subprojects
add_subdirectory(tests)
add_subdirectory(examples)
