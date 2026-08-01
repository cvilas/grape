# =================================================================================================
# Copyright (C) 2025 grape contributors
# =================================================================================================

define_module_example(
  NAME hello
  SOURCES hello.cpp
  PRIVATE_LINK_LIBS PkgConfig::LIBCAMERA)

define_module_example(
  NAME list
  SOURCES list.cpp
  PUBLIC_LINK_LIBS PkgConfig::LIBCAMERA grape::log)
