--[[
Camera image publisher configuration
]]

local hostname = "unknown-host"
local p = io.popen("hostname")
if p then
    local line = p:read("*l") or ""
    p:close()
    line = line:gsub("^%s+", ""):gsub("%s+$", "")
    if line ~= "" then
        hostname = line
    end
end

camera_name="Kiyo"               -- 'hint'. partial name works
pub_topic= hostname .. "/camera" -- IPC topic on which image data is published
compression_speed=1              -- range [1(max compression), 65537(max speed)]
