-- An engine imparts force to the vehicle by any means.
-- The engine interface is also needed to interact with systems such as
-- RCS controllers which may need to know nozzle position, direction
-- and thrust.
local engine = create_interface()

-- (self) -> glm.vec3
-- Returns the position of the nozzle in world coordinates
-- Called on update only
engine.get_nozzle_position = nil

-- (self) -> glm.vec3
-- Returns the forward vector of the nozzle in world coordinates
-- Forward means "fire" direction, not thrust force direction
-- Called on physics_update only once
engine.get_nozzle_forward = nil

-- (self) -> Piece
-- Returns the piece which generates the thrust, the one which contains the 
-- nozzle marker
-- Called on physics_update only once
engine.get_piece = nil

-- OPTIONAL
-- (self, glm.vec3 final_thrust) -> void
-- Useful for implementing visual effects
-- Called on physics_update only once
engine.after_thrust = nil

-- (self, dt) -> float
-- Returns the thrust generated in newtons. Simulate the engine here
-- Called on physics_update only once, result stored for further queries
engine.get_thrust = nil

-- You must manually call this function so all functionality is implemented
function engine:physics_update(dt)
	local f_thrust = self:get_thrust(dt)

	if f_thrust > 0 then
		local piece = self:get_piece()
		local rdir = self:get_nozzle_forward()
		local rpos = self:get_nozzle_position()

		local final_thrust = rdir * f_thrust;

		piece.rigid_body:apply_force(rdir * f_thrust, rpos)

		if self.after_thrust then self:after_thrust(final_thrust) end

	end

end

return engine;