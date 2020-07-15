require("vehicle") 
require("bullet")
require("toml")
local glm = require("glm")
local debug_drawer = require("debug_drawer")

local engine = machine:load_interface("core:interfaces/engine.lua")
engine.thrust = machine.init_toml:get_number("thrust")

local nozzle = machine.init_toml:get_string("nozzle")
local nozzle_dir = nil
local nozzle_pos = nil

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

function engine:get_piece()
	return part:get_piece("p_root")
end

function pre_update(dt)
	if nozzle_dir == nil then 
		nozzle_dir = part:get_piece("p_root"):get_marker_forward(nozzle)
		nozzle_pos = part:get_piece("p_root"):get_marker_position(nozzle)
	end 
end

function update(dt)
	engine:update()
end

