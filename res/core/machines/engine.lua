require("vehicle") 					-- We don't assign it to anything, it just defines user types
local bullet = require("bullet")
local glm = require("glm")
local logger = require("logger")
local toml = require("toml")

local thrust = machine.init_toml:get_number("thrust")
thrust = 0.0

function update(dt)

	local p_root = part:get_piece("p_root")
	local dir = glm.vec3.new(0.0, 0.0, 1.0)
	dir = p_root:transform_axis(dir)
	local pos = p_root:get_relative_position()

	p_root.rigid_body:apply_impulse(dir * thrust, pos)
end
	
