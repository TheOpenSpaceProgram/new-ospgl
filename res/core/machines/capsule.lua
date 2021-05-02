require("vehicle")
require("universe")
local flight_input = require("flight_input")

local input_ctx = flight_input.context.new("core:input/rocket.toml")

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

	local all_throttleable = machine:get_wired_interfaces("core:interfaces/throttleable.lua")
	for _, throttleable in all_throttleable:pairs() do
		throttleable.throttle = in_throttle
	end

	if input_ctx:get_action_down("stage") then 
		local all_activables = machine:get_wired_interfaces("core:interfaces/activable.lua")
		for _, activable in all_activables:pairs() do 
			activable:activate()
		end
	end

	t = t + dt
end

print("Requiring nanovg");
local nvg = require("nano_vg")
