-- This file includes functions for easily filling up CameraUniforms
-- by just giving a set of easy to build vectors
local rnd = require("renderer")
local glm = require("glm")
local logger = require("logger")

local cameras = {}

---@param pos glm.vec3
---@param up glm.vec3
---@param dir glm.vec3
---@param fov number
---@param w number
---@param h number
function cameras.from_pos_and_dir(pos, up, dir, fov, w, h)
    local result = rnd.camera_uniforms.new()

    local proj = glm.perspective(math.rad(fov), w / h, 1e-6, 1e16)
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

-- Angles assumed to be in radians, except fov which is in degrees
--- @param center glm.vec3
--- @param pole glm.vec3
--- @param azimuth number in radians
--- @param altitude number in radians
--- @param radius number
--- @param fov number in degrees
--- @param w integer
--- @param h integer
function cameras.from_center_and_polar(center, pole, azimuth, altitude, radius, fov, w, h)
  local pos = glm.spherical_to_euclidean_r1(azimuth, altitude)
  local view_dir = -pos
  pos = pos * radius
  -- Transform the vector (rotate to match pole, scale by radius, then translate to center)
  -- Remember, matrices are transformed in the opposite order you want the operations to apply!
  local mat = glm.translate(glm.mat4.new(1.0), center)
  mat = mat * glm.rotate_from_to(glm.vec3.new(0, 1, 0), pole)
  pos = glm.vec3.new(mat * glm.vec4.new(pos, 1.0))

  return cameras.from_pos_and_dir(pos, pole, view_dir, fov, w, h)
end

return cameras
