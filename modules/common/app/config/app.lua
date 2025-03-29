--[[===============================================================================================
 Copyright (C) 2024 GRAPE Contributors
=================================================================================================]]

--[[Top level application configuration script in Lua programming language syntax ]]

-- Logger configuration
logger = {
  severity_threshold = "Debug",     -- See grape::log::Severity
}

-- IPC (inter-process communication) configuration
ipc = {
  scope = "Host",                   -- See grape::ipc::Config::Scope 
}
