--- Allows an orbit view around a centered object
--- The orbit view is locked to slightly below the poles of the object
--- Poles can either be solar-system y-up, or element y-up
local cameras = dofile("core:scenes/camera_util.lua")
local glm = require("glm")
local logger = require("logger")
local input = require("input")

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
map_camera.x_speed = 0.0
map_camera.y_speed = 0.0
map_camera.zoom_speed = 0.0
map_camera.radius = 30000e3
map_camera.fov = 40.0

map_camera.sensitivity = 0.0
map_camera.inertia_damp = 4.0
map_camera.zoom_damp = 12.0
---@type function|nil
--- If present, it will be called if focused entity disappears with the camera as first argument
--- You must set map_camera.center_element or map_camera.center_entity appropiately 
map_camera.on_focus_lost = nil

function map_camera:update(dt) 
  
  self.zoom_speed = self.zoom_speed + input.get_scroll()
  self.radius = self.radius * (1.0 - self.zoom_speed * dt * 0.5)
  self.radius = math.max(self.radius, self:get_min_radius())

  if self.zoom_speed > 0 then
    self.zoom_speed = self.zoom_speed - self.zoom_damp * dt
    if self.zoom_speed < 0 then self.zoom_speed = 0 end
  elseif self.zoom_speed < 0 then
    self.zoom_speed = self.zoom_speed + self.zoom_damp * dt
    if self.zoom_speed > 0 then self.zoom_speed = 0 end
  end

  if input.mouse_pressed(input.btn.left) then 
    self.azimuth = self.azimuth + input.get_mouse_delta().x * self.sensitivity
    self.altitude = self.altitude - input.get_mouse_delta().y * self.sensitivity
    self.x_speed = input.get_mouse_delta().x * self.sensitivity * 10.0
    self.y_speed = -input.get_mouse_delta().y * self.sensitivity * 10.0
  else
    if self.x_speed > 0 then
      self.x_speed = self.x_speed - self.inertia_damp * dt
      if self.x_speed < 0 then self.x_speed = 0 end
    elseif self.x_speed < 0 then
      self.x_speed = self.x_speed + self.inertia_damp * dt
      if self.x_speed > 0 then self.x_speed = 0 end
    end
    if self.y_speed > 0 then
      self.y_speed = self.y_speed - self.inertia_damp * dt
      if self.y_speed < 0 then self.y_speed = 0 end
    elseif self.y_speed < 0 then
      self.y_speed = self.y_speed + self.inertia_damp * dt
      if self.y_speed > 0 then self.y_speed = 0 end
    end

    self.azimuth = self.azimuth + self.x_speed * dt
    self.altitude = self.altitude + self.y_speed * dt
  end
    
  self.altitude = glm.clamp(self.altitude, 0.01, math.pi - 0.01)
end

function map_camera:get_center()
  if self.center_element then 
    return self.universe.system:get_element_position(self.center_element)
  else
    local ent = self.universe:get_entity(self.center_entity)
    if ent then 
      return ent:get_visual_origin()
    else
      self.center_entity = nil
      self.center_element = "__default"

      if self.on_focus_lost then 
        self:on_focus_lost()
      end
      
      -- Recursive call, as it should be properly set now
      return map_camera:get_center()
    end
  end
end

function map_camera:get_min_radius()
  if self.center_element then 
    return self.universe.system:get_element(self.center_element).config.radius * 3.0
  end
  
  return 1000.0
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
