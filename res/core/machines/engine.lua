require("vehicle") 
require("bullet")
require("toml")
local glm = require("glm")
local debug_drawer = require("debug_drawer")

local thrust = machine.init_toml:get_number("thrust")
local throttle = 0.0
local nozzle = machine.init_toml:get_string("nozzle")
local nozzle_dir = nil
local nozzle_pos = nil
local engine = machine:load_interface("core:interfaces/engine.lua")

engine.wow = "WOw"

function engine:get_nozzle_position()
	local p_root = part:get_piece("p_root")
	-- Position is relative to the center of mass of the rigidbody
	return p_root:transform_point_to_rigidbody(nozzle_pos)
end

function engine:get_nozzle_forward()
	local p_root = part:get_piece("p_root")
	-- Direction is on world coordinates, the nozzle indicates "fire" direction, not thrust direction
	return -p_root:transform_axis(nozzle_dir)
end

function pre_update(dt)
	if nozzle_dir == nil then 
		nozzle_dir = part:get_piece("p_root"):get_marker_forward(nozzle)
		nozzle_pos = part:get_piece("p_root"):get_marker_position(nozzle)
	end 
	engine:say_wow();
end

function update(dt)
	local f_thrust = thrust * engine.throttle 

	if f_thrust > 0 then

		local p_root = part:get_piece("p_root")
		local rdir = engine:get_nozzle_forward()
		local rpos = engine:get_nozzle_position()

		p_root.rigid_body:apply_force(rdir * f_thrust, rpos)

		-- Draw flame
		local fpos = glm.vec3.new(p_root:get_graphics_transform():to_mat4() * glm.vec4.new(nozzle_pos, 1.0))
		debug_drawer.add_cone(fpos, fpos - rdir * 10.0 * engine.throttle, 0.5, glm.vec3.new(1.0, 1.0, 0.0))

	end

end

