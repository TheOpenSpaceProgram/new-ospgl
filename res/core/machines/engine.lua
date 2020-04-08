require("vehicle") 
local bullet = require("bullet")
local glm = require("glm")
local logger = require("logger")
local toml = require("toml")
local debug_drawer = require("debug_drawer")

local thrust = machine.init_toml:get_number("thrust")
local throttle = 0.0
local nozzle = machine.init_toml:get_string("nozzle")

local nozzle_dir = nil
local nozzle_pos = nil

function set_throttle(port, value)
	
	throttle = math.max(math.min(value, 1.0), 0.0)

end 

function define_ports()
	machine:add_input_port("Throttle", "number", set_throttle) 
end

function update(dt)
	local f_thrust = thrust * throttle 

	if nozzle_dir == nil then 
		nozzle_dir = part:get_piece("p_root"):get_marker_forward(nozzle)
		nozzle_pos = part:get_piece("p_root"):get_marker_position(nozzle)
	end 

	if f_thrust > 0 then

		local p_root = part:get_piece("p_root")
		-- Direction is on world coordinates
		local rdir = -p_root:transform_axis(nozzle_dir)
		-- Position is relative to the center of mass of the rigidbody
		local rpos = p_root:transform_point_to_rigidbody(nozzle_pos)

		p_root.rigid_body:apply_force(rdir * thrust * throttle, rpos)

		-- Draw flame
		local fpos = glm.vec3.new(p_root:get_graphics_transform():to_mat4() * glm.vec4.new(nozzle_pos, 1.0))
		debug_drawer.add_cone(fpos, fpos - rdir * 10.0 * throttle, 0.5, glm.vec3.new(1.0, 1.0, 0.0))

	end

end
	
