# =================================================================================================
# Copyright (C) 2025 GRAPE Contributors
# =================================================================================================

define_module_pybinding(
  NAME
  grape_ipc_py
  SOURCES
  bindings.h
  bindings.cpp
  config_bindings.cpp
  match_bindings.cpp
  publisher_bindings.cpp
  session_bindings.cpp
  subscriber_bindings.cpp)
