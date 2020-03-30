require("vehicle") 					-- We don't assign it to anything, it just defines user types
local bullet = require("bullet")
local glm = require("glm")
local logger = require("logger")

function update(dt)

	local p_root = part:get_piece("p_root")
	local dir = glm.vec3.new(0.0, 0.0, 1.0)
	dir = p_root:transform_axis(dir)
	local thrust = 10.0
	local pos = p_root:get_relative_position()

	p_root.rigid_body:apply_impulse(dir * thrust, pos)
end
	
