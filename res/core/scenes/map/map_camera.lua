--- Allows an orbit view around a centered object
--- The orbit view is locked to slightly below the poles of the object
--- Poles can either be solar-system y-up, or element y-up
local cameras = dofile("core:scenes/camera_util.lua")
local glm = require("glm")

local map_camera = {}
  
--- May be an integer, then it's assumed to be a solar system element, or 
--- an entity, in which case the entity is followed
map_camera.center = 0
--- If true, we follow the center object pole. Otherwise we use the universal pole (y up)
map_camera.center_pole = false
map_camera.default_pole = glm.vec3.new(0, 1, 0)

map_camera.azimuth = 0.0
map_camera.altitude = 0.0
map_camera.radius = 0.0
map_camera.fov = 70.0

map_camera.sensitivity = 0.0
map_camera.tremendous = "hello, world!"


function map_camera:update() 

end

function map_camera:get_center()
  return glm.vec3.new(0, 0, 0)
end

function map_camera:get_pole()
  if self.center_pole then 

  else
    return self.default_pole
  end
end

function map_camera:get_camera_uniforms(width, height)
  return cameras.from_center_and_polar(self:get_center(), self:get_pole(), 
    self.azimuth, self.altitude, self.radius, self.fov, width, height)
end

---@param universe universe
function map_camera:init(universe)
  self.universe = universe
  return self
end

return map_camera
