require("vehicle")
require("universe")

local logger = require("logger")
local toml = require("toml")

local ev_handler = nil

function define_ports()
	machine:add_output_port("Throttle", "number")
	ev_handler = universe:sign_up_for_event("test", test)

end

local t = 0.0

function test(number)

	logger.info("Received event number " .. number)

end

function update(dt)

	--machine:write_to_port("Throttle", math.abs(math.sin(t)))
	machine:write_to_port("Throttle", 0.0)

	universe:emit_event("test", 10)

	t = t + dt
end
	
