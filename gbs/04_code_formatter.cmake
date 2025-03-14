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

add_custom_target(format COMMENT "Format source files")

# Function to apply clang formatting on a target
function(add_clang_format target_name)
  if(CLANG_FORMAT_BIN AND ENABLE_FORMATTER)
    if(NOT TARGET ${target_name})
      message(FATAL_ERROR "add_clangformat called on a non-target \"${target_name}\"")
    endif()

    # figure out which sources this should be applied to
    get_target_property(_clang_sources ${target_name} SOURCES)
    get_target_property(_builddir ${target_name} BINARY_DIR)

    # There are file types we don't want to process (or may be the inverse list is shorter)
    set(supported_file_types ".c" ".cpp" ".h" ".hpp" ".inl")

    set(sources "")
    foreach(source ${_clang_sources})
      if(NOT TARGET ${source})
        get_filename_component(_ext_type ${source} EXT)
        if(NOT ${_ext_type} STREQUAL "")
          if(${_ext_type} IN_LIST supported_file_types)
            get_filename_component(_source_file ${source} NAME)
            get_source_file_property(_file_loc "${source}" LOCATION)

            math(EXPR counter "${counter}+1")
            set(format_file
                .${CMAKE_FILES_DIRECTORY}/${target_name}_${counter}_${_source_file}.format)
            add_custom_command(
              OUTPUT ${format_file}
              DEPENDS ${source}
              COMMENT "Format ${source}"
              COMMAND ${CLANG_FORMAT_BIN} -style=file -i ${_file_loc}
              COMMAND ${CMAKE_COMMAND} -E touch ${format_file})
            list(APPEND sources ${format_file})
          endif() # in included types list
        endif() # extension detected
      endif()
    endforeach()

    if(sources)
      add_custom_target(
        ${target_name}_clangformat
        SOURCES ${sources}
        COMMENT "Format target ${_target}")
      add_dependencies(${target_name} ${target_name}_clangformat)
      add_dependencies(format ${target_name}_clangformat)
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
