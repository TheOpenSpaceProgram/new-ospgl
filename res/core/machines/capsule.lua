require("vehicle")
require("universe")

local logger = require("logger")
local toml = require("toml")

local ev_handler = nil

function ev_handler_fnc()
	print(":)")
end

function define_ports()
	machine:add_output_port("Throttle", "number")

	ev_handler universe:sign_up_for_event("test", ev_handler_fnc)
end

local t = 0.0


function update(dt)

	--machine:write_to_port("Throttle", math.abs(math.sin(t)))
	machine:write_to_port("Throttle", 0.0)

	universe:emit_event("test", 1.0)

	t = t + dt
end
	
