local engine = {}

engine.throttle = 0.0

-- (self) -> glm.vec3
-- Returns the position of the nozzle in world coordinates
-- Called on update only
engine.get_nozzle_position = nil

-- (self) -> glm.vec3
-- Returns the forward vector of the nozzle in world coordinates
-- Forward means "fire" direction, not thrust force direction
-- Called on update only
engine.get_nozzle_forward = nil

return engine;