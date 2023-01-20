--- Allows an orbit view around a centered object
--- The orbit view is locked to slightly below the poles of the object
--- Poles can either be solar-system y-up, or element y-up
local cameras = dofile("core:scenes/camera_util.lua")

local map_camera = {}

function map_camera.get_camera_uniforms(width, height)

    return cameras.from_pos_and_dir(glm.vec3.new(-1000, 0, 0),
        glm.vec3.new(0, 1, 0), -glm.normalize(offset), 50.0, renderer:get_size())
    

end

return map_camera
