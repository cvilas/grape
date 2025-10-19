# =================================================================================================
# Copyright (C) 2025 grape contributors
# =================================================================================================

define_module_example(
  NAME example
  SOURCES example.cpp
  PRIVATE_LINK_LIBS PkgConfig::LIBCAMERA)
