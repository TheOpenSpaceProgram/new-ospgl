require("vehicle")
require("universe")
local flight_input = require("flight_input")

local input_ctx = flight_input.context.new("input/rocket.toml")

function get_input_context()
	return input_ctx
end 

local t = 0.0

-- pre_update is used because we manage inputs
function pre_update(dt)

	if input_ctx:get_action_down("cut_throttle") then 
		input_ctx:set_axis("throttle", -1.0, 0.0)
	end 

	if input_ctx:get_action_down("full_throttle") then 
		input_ctx:set_axis("throttle", 1.0, 0.0)
	end

	-- Axis goes from -1 to 1, we need to go from 0 to 1
	local in_throttle = (input_ctx:get_axis("throttle") + 1.0) * 0.5
	--machine:write_to_port("Throttle", in_throttle)

	--machine:write_to_port("Pitch", input_ctx:get_axis("pitch"))
	--machine:write_to_port("Yaw", input_ctx:get_axis("yaw"))
	--machine:write_to_port("Roll", input_ctx:get_axis("roll"))

	t = t + dt
end
	
