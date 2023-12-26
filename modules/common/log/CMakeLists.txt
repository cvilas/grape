#=================================================================================================
# Copyright (C) 2018-2023 GRAPE Contributors
#=================================================================================================

declare_module(NAME log DEPENDS_ON_MODULES "utils" DEPENDS_ON_EXTERNAL_PROJECTS "")

# library sources
set(SOURCES
        README.md
        include/grape/log/logger.h
        include/grape/log/singleton_logger.h)

# library target
define_module_library(
        NAME log
        PUBLIC_LINK_LIBS grape::utils
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
