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
  mode = "client",
  router = "tcp/127.0.0.1:7447",  -- router locator
  topic_prefix = ""               -- Prefix to apply to topics.
}
