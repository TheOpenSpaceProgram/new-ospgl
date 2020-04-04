require("vehicle")
local logger = require("logger")
local toml = require("toml")

function define_ports()
	machine:add_output_port("Throttle", "number")
end

local t = 0.0

function update(dt)

	machine:write_to_port("Throttle", math.abs(math.sin(t)))


	t = t + dt
end
	
