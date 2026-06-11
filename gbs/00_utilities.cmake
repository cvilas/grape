# =================================================================================================
# Copyright (C) 2024 GRAPE Contributors
# =================================================================================================

#==================================================================================================
# Print all variables generated during configuration
function(print_cmake_variables)
  get_cmake_property(variable_names VARIABLES)
  list (SORT variable_names)
  foreach (var ${variable_names})
    message(STATUS "${var}=${${var}}")
  endforeach()
endfunction()

include(CheckCXXSourceCompiles)

#==================================================================================================
# Detect whether Git LFS files in external/sources/ have been downloaded.
# (An LFS pointer file always starts with "version https://git-lfs.github.com/spec/v1" (48 bytes))
function(check_lfs_pulled)
  file(GLOB _archives CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/external/sources/*.tar.gz")
  foreach(_archive IN LISTS _archives)
    file(READ "${_archive}" _header LIMIT 50)
    if(_header MATCHES "^version https://git-lfs")
      message(FATAL_ERROR
        "Git LFS source archives have not been downloaded.\n"
        "Please run 'git lfs pull' and re-run CMake.")
    endif()
  endforeach()
endfunction()

#==================================================================================================
# Detect which standard library is in use
# USING_LIBSTDCXX is set if libstdc++ is in use
# USING_LIBCPP is set if libc++ is in use
function(detect_cxx_stdlib)
    # Save the current compiler flags
    set(CMAKE_REQUIRED_FLAGS_SAVED ${CMAKE_REQUIRED_FLAGS})
    
    # Test for libstdc++
    set(CMAKE_REQUIRED_FLAGS "${CMAKE_CXX_FLAGS}")
    check_cxx_source_compiles("
        #include <cstddef>
        #if !defined(__GLIBCXX__)
        #error Not using libstdc++
        #endif
        int main() { return 0; }
    " USING_LIBSTDCXX)
    
    # Test for libc++
    check_cxx_source_compiles("
        #include <cstddef>
        #if !defined(_LIBCPP_VERSION)
        #error Not using libc++
        #endif
        int main() { return 0; }
    " USING_LIBCPP)
    
    # Restore compiler flags
    set(CMAKE_REQUIRED_FLAGS ${CMAKE_REQUIRED_FLAGS_SAVED})
    
    # Set results in parent scope
    set(USING_LIBSTDCXX ${USING_LIBSTDCXX} PARENT_SCOPE)
    set(USING_LIBCPP ${USING_LIBCPP} PARENT_SCOPE)
endfunction()
