require("vehicle")
require("universe")
local flight_input = require("flight_input")
local logger = require("logger")
local toml = require("toml")

local input_ctx = flight_input.context.new("input/rocket.toml")

local throttle

function get_input_context()
	return input_ctx
end 

function define_ports()
	machine:add_output_port("Throttle", "number")

end

local t = 0.0


function update(dt)

	if input_ctx:get_action_down("cut_throttle") then 
		input_ctx:set_axis("throttle", -1.0, 0.0)
	end 

	if input_ctx:get_action_down("full_throttle") then 
		input_ctx:set_axis("throttle", 1.0, 0.0)
	end

	-- Axis goes from -1 to 1
	local in_throttle = (input_ctx:get_axis("throttle") + 1.0) * 0.5
	logger.info("Throttle set to: " .. in_throttle)
	machine:write_to_port("Throttle", in_throttle)

	t = t + dt
end
	
