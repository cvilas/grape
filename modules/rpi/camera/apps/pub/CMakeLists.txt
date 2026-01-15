# =================================================================================================
# Copyright (C) 2025 GRAPE Contributors
# =================================================================================================

define_module_app(
  NAME rpi_camera_pub
  SOURCES rpi_camera_pub.cpp
  PUBLIC_LINK_LIBS "grape::conio;grape::script")

set(CONF_DEST_DIR ${CMAKE_PROJECT_NAME}/rpi_camera_pub)
configure_file(config.lua ${CMAKE_BINARY_DIR}/share/${CONF_DEST_DIR}/config.lua)
install(FILES config.lua DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/${CONF_DEST_DIR})
