# =================================================================================================
# Copyright (C) 2023 GRAPE Contributors
# =================================================================================================

define_module_test(NAME tests SOURCES utils_tests.cpp enum_tests.cpp ip_tests.cpp
                                      file_system_tests.cpp)

# For testing getSystemName
add_dependencies(grape_utils_tests grape_show_system_name)
