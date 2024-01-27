#=================================================================================================
# Copyright (C) 2023 GRAPE Contributors
#=================================================================================================

define_module_test(
        NAME tests
        SOURCES program_args_tests.cpp
        PUBLIC_INCLUDE_PATHS
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
        PUBLIC_LINK_LIBS "")
