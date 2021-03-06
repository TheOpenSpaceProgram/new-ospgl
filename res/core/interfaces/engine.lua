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
-- Called on update only
engine.get_nozzle_forward = nil

-- (self) -> Piece
-- Returns the piece which generates the thrust, the one which contains the 
-- nozzle marker
-- Called on update only
engine.get_piece = nil

-- OPTIONAL
-- (self, glm.vec3 final_thrust) -> void
-- Useful for implementing visual effects
engine.after_thrust = nil

-- (self) -> float
-- Returns the thrust generated in newtons
-- Called on update only
engine.get_thrust = nil


function engine:update()
	local f_thrust = self:get_thrust()

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