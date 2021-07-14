local plumbing = {};

require("toml")
local logger = require("logger")
local nvg = require("nano_vg")
require("game_database")

function plumbing.is_requester() return true end

function plumbing.fluid_update()

end

function plumbing.get_pressure(port)

end

function plumbing.out_flow(port, volume)

end

function plumbing.in_flow(port, fluids)

end

function plumbing.get_free_volume()

end

function plumbing.draw_diagram(vg)

    nvg.begin_path(vg)
    nvg.rounded_rect(vg, 0, 0, 3, 3, 0.5)
    nvg.fill(vg)
    nvg.stroke(vg)

end

local port_str = database:get_string("inlet")
function plumbing.init(ports)
    local port_marker = machine.init_toml:get_string("port_marker")
    machine.plumbing:create_port("port", port_marker, port_str, 1.5, 3)

    return 3, 3
end

return plumbing;
