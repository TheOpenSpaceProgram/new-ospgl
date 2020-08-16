
require("vehicle")
require("universe")
local logger = require("logger")
require("toml")

local pitch = 0.0
local yaw = 0.0
local roll = 0.0


local engines = nil 

function init() 
	engines = load_engines(machine.init_toml)
end 

local function in_axis(axis, value)
	if axis == "Pitch" then
		pitch = value 
	elseif axis == "Yaw" then
		yaw = value 
	elseif axis == "Roll" then
		roll = value 
	end

	-- This guarantees zero latency if all inputs are set from
	-- pre_update (this happens for player input)
	if machine:all_inputs_ready() then 
		write_to_engines()
	end

end

function define_ports()
	machine:add_input_port("Pitch", "number", in_axis)
	machine:add_input_port("Yaw", "number", in_axis)
	machine:add_input_port("Roll", "number", in_axis)
end

-- Quite complex, we "simulate" the effect each engine has on the
-- vessel, if it's wanted we turn on that engine, otherwise we turn if off
local function write_to_engines()

end

local function load_engines(toml)
	local list = toml:get_array_of_string("engines")

	local engines = {}
	for idx, v in ipairs(list) do 
		engines[idx] = part:get_machine(v)
	end 

	logger.info("RCS controller received ".. #engines .. " engines")

	return engines

end

