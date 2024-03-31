# =================================================================================================
# Copyright (C) 2018 GRAPE Contributors
# =================================================================================================

# --------------------------------------------------------------------------------------------------
# Configures source code formatting tools.Apply by calling add_clang_format() on target
# --------------------------------------------------------------------------------------------------
option(ENABLE_FORMATTER "Enable automatic source file formatting" ON)

# Enable code formatting
if(ENABLE_FORMATTER)
  find_program(CLANG_FORMAT_BIN NAMES clang-format QUIET)
  if(NOT CLANG_FORMAT_BIN)
    message(WARNING "Code formatter (clang-format) requested but not found")
  endif()
endif()

# Target to format source files
add_custom_target(format)

# Function to apply clang formatting on a target
# https://www.linkedin.com/pulse/simple-elegant-wrong-how-integrate-clang-format-friends-brendan-drew

function(add_clang_format _target_name)
  if(CLANG_FORMAT_BIN AND ENABLE_FORMATTER)
    if(NOT TARGET ${_target_name})
      message(FATAL_ERROR "add_clangformat called on a non-target \"${_target_name}\"")
    endif()

    # figure out which sources this should be applied to
    get_target_property(_clang_sources ${_target_name} SOURCES)
    get_target_property(_builddir ${_target_name} BINARY_DIR)

    # There are file types we don't want to process (or may be the inverse list is shorter)
    set(_supported_file_types ".c" ".cpp" ".h" ".hpp" ".inl")

    set(_sources "")
    foreach(_source ${_clang_sources})
      if(NOT TARGET ${_source})
        get_filename_component(_ext_type ${_source} EXT)
        if(NOT ${_ext_type} STREQUAL "")
          if(${_ext_type} IN_LIST _supported_file_types)
            get_filename_component(_source_file ${_source} NAME)
            get_source_file_property(_file_loc "${_source}" LOCATION)

            math(EXPR counter "${counter}+1")
            set(_format_file
                .${CMAKE_FILES_DIRECTORY}/${_target_name}_${counter}_${_source_file}.format)
            add_custom_command(
              OUTPUT ${_format_file}
              DEPENDS ${_source}
              COMMENT "Format ${_source}"
              COMMAND ${CLANG_FORMAT_BIN} -style=file -i ${_file_loc}
              COMMAND ${CMAKE_COMMAND} -E touch ${_format_file})
            list(APPEND _sources ${_format_file})
          endif() # in included types list
        endif() # extension detected
      endif()
    endforeach()

    if(_sources)
      add_custom_target(
        ${_target_name}_clangformat
        SOURCES ${_sources}
        COMMENT "Format target ${_target}")
      add_dependencies(${_target_name} ${_target_name}_clangformat)
      add_dependencies(format ${_target_name}_clangformat)
    endif()
  endif() # CLANG_FORMAT_BIN found
endfunction()

# Enable cmake file formatting and add to 'format' target
option(ENABLE_CMAKE_FORMATTER "Enable automatic CMake file formatting" ON)
if(ENABLE_CMAKE_FORMATTER)
  find_program(CMAKE_FORMAT_BIN NAMES cmake-format QUIET)
  if(NOT CMAKE_FORMAT_BIN)
    message(WARNING "CMake formatter (cmake-format) requested but not found")
  else()
    add_custom_target(
      format-cmake
      COMMENT "Format CMake files"
      COMMAND find . -name '*.cmake' -o -name 'CMakeLists.txt' | xargs cmake-format -i
      WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/modules)
    add_dependencies(format format-cmake)
  endif()
endif()

# print summary
message(STATUS "\tENABLE_FORMATTER       : ${ENABLE_FORMATTER} (${CLANG_FORMAT_BIN})")
message(STATUS "\tENABLE_CMAKE_FORMATTER : ${ENABLE_FORMATTER} (${CMAKE_FORMAT_BIN})")
