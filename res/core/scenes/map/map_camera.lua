--- Allows an orbit view around a centered object
--- The orbit view is locked to slightly below the poles of the object
--- Poles can either be solar-system y-up, or element y-up
local glm = require("glm")

-- We modify an orbit camera

local map_camera = dofile("core:scenes/orbit_camera.lua")

---@type string|nil
--- If nil, then map_camera.center_entity must be present and be a number (entity id)
--- If string is "__default", then the first system element will be focused, usually the star
map_camera.center_element = "Earth"
---@type number|nil
map_camera.center_entity = nil
--- If true, we follow the center object pole. Otherwise we use the universal pole (y up)
map_camera.center_pole = false
map_camera.default_pole = glm.vec3.new(0, 1, 0)

---@type function|nil
--- If present, it will be called if focused entity disappears with the camera as first argument
--- You must set map_camera.center_element or map_camera.center_entity appropiately
map_camera.on_focus_lost = nil


function map_camera:get_center()
	if self.center_element then
		return self.universe.system:get_element_position(self.center_element)
	else
		local ent = self.universe:get_entity(self.center_entity)
		if ent then
			return ent:get_position()
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
		return glm.vec3.new(1, 0, 0)
	else
		return self.default_pole
	end
end


---@param universe universe
---@param gui_input gui.input
function map_camera:init(universe, gui_input)
	self.universe = universe
	self.gui_input = gui_input
	return self
end

return map_camera
