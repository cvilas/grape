--[[===============================================================================================
 Copyright (C) 2024 GRAPE Contributors
=================================================================================================]]

--[[Template top-level GRAPE application configuration script in Lua programming language syntax ]]

-- Logger configuration
logger = {
  severity_threshold = "Debug",     -- See grape::log::Severity for options
}

-- IPC (inter-process communication) configuration
ipc = {
  mode = "Client", -- Can be 'Client', 'Peer'
  router = "tcp/[::]:7447", -- locator of router to connect to
}
