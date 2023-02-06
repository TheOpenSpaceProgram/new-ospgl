local glm = require("glm")
local logger = require("logger")

local flight_camera = dofile("core:scenes/orbit_camera.lua")

---@type universe.entity|nil
flight_camera.tracked_veh = nil

---@type integer
--- Piece (ID) we orbit around. If <= 0, we orbit the C.O.M (default)
flight_camera.tracked_piece = 0

---@type integer
--- Piece (ID) used as a reference for pole alignment, typically the controlled capsule
--- Defaults to root piece
flight_camera.reference_piece = 1

---@type string
--- "chase": Pole is aligned with vehicle / piece forward direction
--- "schase": Pole is aligned with vehicle  / piece forward direction, smoothed
--- "ref": Pole is aligned with frame of reference pole
--- "surf": Pole is aligned with closest planet surface normal vector
flight_camera.mode = "chase"

---@type boolean
--- If true, the chase modes will be aligned with the focused piece instead of reference
flight_camera.chase_piece = false

function flight_camera:get_center()
	if not self.tracked_veh then
		return glm.vec3.new(0, 0, 0)
	end
		
	if self.tracked_piece <= 0 then
		return self.tracked_veh:get_position()
	else
		---@type vehicle
		local veh = self.tracked_veh.lua.vehicle
		local piece = veh:get_piece_by_id(self.tracked_piece)
		if piece then
			return piece:get_graphics_position()	
		else
			--- Refocus COM, piece was lost
			self.tracked_piece = 0
			return self:get_center()
		end
	end

end

function flight_camera:get_min_radius()
	return 10.0
end

function flight_camera:get_pole()
	if not self.tracked_veh then
		return glm.vec3.new(0, 1, 0)
	end
		
	if self.mode == "chase" then
		return self.tracked_veh:get_orientation() * glm.vec3.new(1, 0, 0)
	elseif self.mode == "schase" then

	elseif self.mode == "ref" then

	else

	end
	return glm.vec3.new(0, 1, 0)
end


---@param universe universe
---@param gui_input gui.input
function flight_camera:init(universe, gui_input)
	self.universe = universe
	self.gui_input = gui_input
	return self
end

return flight_camera
