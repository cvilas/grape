# https://www.linkedin.com/pulse/simple-elegant-wrong-how-integrate-clang-format-friends-brendan-drew

function(add_clang_format _targetname)
    if (CLANG_FORMAT_FOUND)
        if (NOT TARGET ${_targetname})
            message(FATAL_ERROR "add_clangformat should only be called on targets (got " ${_targetname} ")")
        endif ()

        # figure out which sources this should be applied to
        get_target_property(_clang_sources ${_targetname} SOURCES)
        get_target_property(_builddir ${_targetname} BINARY_DIR)

        # There are file types we don't want to process (or may be the inverse list is shorter)
        set(_supported_file_types ".c" ".cpp" ".h" ".hpp")

        set(_sources "")
        foreach (_source ${_clang_sources})
            if (NOT TARGET ${_source})
                get_filename_component(_ext_type ${_source} EXT)
                if(NOT ${_ext_type} STREQUAL "")
                    if(${_ext_type} IN_LIST _supported_file_types)
                        get_filename_component(_source_file ${_source} NAME)
                        get_source_file_property(_clang_loc "${_source}" LOCATION)

                        set(_format_file ${_targetname}_${_source_file}.format)

                        add_custom_command(OUTPUT ${_format_file}
                                DEPENDS ${_source}
                                COMMENT "Clang-Format ${_source}"
                                COMMAND ${CLANG_FORMAT_BIN} -style=file -fallback-style=Google -sort-includes -i ${_clang_loc}
                                COMMAND ${CMAKE_COMMAND} -E touch ${_format_file})

                        list(APPEND _sources ${_format_file})
                    endif() # in included types list
                endif() # extension detected
            endif ()
        endforeach ()

        if (_sources)
            add_custom_target(${_targetname}_clangformat
                    SOURCES ${_sources}
                    COMMENT "Clang-Format for target ${_target}")

            add_dependencies(${_targetname} ${_targetname}_clangformat)
        endif ()

    endif (CLANG_FORMAT_FOUND)
endfunction()
