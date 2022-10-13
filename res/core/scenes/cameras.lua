-- This file includes functions for easily filling up CameraUniforms
-- by just giving a set of easy to build vectors
local rnd = require("renderer")

local cameras = {}

function cameras.from_pos_and_dir(pos, dir)
    local result = rnd.camera_uniforms.new()


    return result
end


return cameras
