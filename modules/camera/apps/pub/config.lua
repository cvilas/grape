--[[
Camera image publisher configuration
]]

camera_name="Kiyo"          -- 'hint'. partial name works
pub_topic="/camera"         -- IPC topic on which image data is published
frame_rate_divisor=1        -- rate limiting factor
image_scale_factor=1.       -- image scaling factor 
compression_speed=1         -- range [1(max compression), 65537(max speed)]
