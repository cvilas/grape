--[[
Camera image publisher configuration
]]

local hostname = os.getenv("HOST") or ""
if hostname == "" then
    local f = io.open("/etc/hostname", "r")
    if f then
        hostname = f:read("*l")  -- first line
        hostname = hostname:gsub("%s+$", "")  -- trim trailing whitespace
        f:close()
    end
end

camera_name="Kiyo"               -- 'hint'. partial name works
pub_topic= hostname .. "/camera" -- IPC topic on which image data is published
compression_speed=1              -- range [1(max compression), 65537(max speed)]
