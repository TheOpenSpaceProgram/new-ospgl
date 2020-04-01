require("vehicle")
local logger = require("logger")
local toml = require("toml")

function define_ports()
	machine:add_output_port("Throttle", "number")
end

function update(dt)

end
	
