-- This file includes functions for easily filling up CameraUniforms
-- by just giving a set of easy to build vectors
local rnd = require("renderer")
local glm = require("glm")

local cameras = {}

function cameras.from_pos_and_dir(pos, up, dir, fov, w, h)
    local result = rnd.camera_uniforms.new()

    local proj = glm.perspective(glm.radians(fov), w / h, 1e-6, 1e16)
    local view = glm.look_at(glm.vec3.new(0, 0, 0), dir, up)
    local proj_view = proj * view 

    result.proj = proj
    result.view = view
    result.proj_view = proj_view
    result.c_model = glm.translate(glm.mat4.new(1.0), -pos)
    result.tform = proj * view * result.c_model
    result.far_plane = 1e16
    result.cam_pos = pos
    result.screen_size = glm.vec2.new(w, h)

    return result
end


return cameras
