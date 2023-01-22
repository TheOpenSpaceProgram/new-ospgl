--- Allows an orbit view around a centered object
--- The orbit view is locked to slightly below the poles of the object
--- Poles can either be solar-system y-up, or element y-up
local cameras = dofile("core:scenes/camera_util.lua")
local glm = require("glm")
local logger = require("logger")

local map_camera = {}
  
---@type string|nil 
--- If nil, then map_camera.center_entity must be present and be a number (entity id)
--- If string is "__default", then the first system element will be focused, usually the star
map_camera.center_element = "Earth"
---@type number|nil
map_camera.center_entity = nil
--- If true, we follow the center object pole. Otherwise we use the universal pole (y up)
map_camera.center_pole = false
map_camera.default_pole = glm.vec3.new(0, 1, 0)

map_camera.azimuth = 0.0
map_camera.altitude = math.pi / 2
map_camera.radius = 30000e3
map_camera.fov = 40.0

map_camera.sensitivity = 0.0

---@type function|nil
--- If present, it will be called if focused entity disappears with the camera as first argument
--- You must set map_camera.center_element or map_camera.center_entity appropiately 
--- Otherwise it may deadlock!
map_camera.on_focus_lost = nil

function map_camera:update() 
end

function map_camera:get_center()
  if self.center_element then 
    return self.universe.system:get_element_position(self.center_element)
  else
    local ent = self.universe:get_entity(self.center_entity)
    if ent then 
      return ent:get_visual_origin()
    else
      if self.on_focus_lost then 
        self:on_focus_lost()
      else
        self.center_entity = nil
        self.center_element = "__default"
      end
      
      -- Recursive call, as it should be properly set now
      return map_camera:get_center()
    end
  end
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
