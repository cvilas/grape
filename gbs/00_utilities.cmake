# =================================================================================================
# Copyright (C) 2024 GRAPE Contributors
# =================================================================================================

#==================================================================================================
# Print all variables generated during configuration
function(print_cmake_variables)
    get_cmake_property(_variable_names VARIABLES)
    list (SORT _variable_names)
    foreach (_var ${_variable_names})
        message(STATUS "${_var}=${${_var}}")
    endforeach()
endfunction()
