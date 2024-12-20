# =================================================================================================
# Copyright (C) 2024 GRAPE Contributors
# =================================================================================================

set(CONFIG_SOURCES config.lua)
foreach(f ${CONFIG_SOURCES})
  configure_file(${f} ${CMAKE_BINARY_DIR}/share/${CMAKE_PROJECT_NAME}/config/${f})
endforeach()
install(FILES ${CONFIG_SOURCES}
        DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/${CMAKE_PROJECT_NAME}/config)
