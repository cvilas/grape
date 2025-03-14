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
