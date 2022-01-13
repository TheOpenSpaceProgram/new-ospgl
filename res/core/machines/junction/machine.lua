local assets = require("assets")
local plumbing_lib = require("plumbing")
local logger = require("logger")
local nvg = require("nano_vg")
require("toml")
require("game_database")
require("vehicle")


local icon = assets.get_image("machines/icons/default_icon.png")
function get_icon() return icon end

plumbing = {}

function plumbing.fluid_update()

end

function plumbing.out_flow(port, volume, do_flow)
    return plumbing_lib.stored_fluids.new()
end

function plumbing.in_flow(port, fluids, do_flow)
    return plumbing_lib.stored_fluids.new()
end

function plumbing.get_pressure_drop(port_a, port_b, cur_P)

end

function plumbing.draw_diagram(vg, skin)
    nvg.begin_path(vg)
    -- Chamber part
    nvg.move_to(vg, 0.5, 0.5)
    nvg.line_to(vg, 1.5, 0.5)
    nvg.move_to(vg, 0.5, 0.5)
    nvg.line_to(vg, 0.5, -0.5)
    nvg.move_to(vg, 0.5, 0.5)
    nvg.line_to(vg, 0.5, 1.5)
    nvg.stroke(vg)

end

-- We create needed ports
local port_str = database:get_string("junction_port")
function plumbing.init()
    machine.plumbing:create_port("port_1", "", port_str, true, 1.5, 0.5)
    machine.plumbing:create_port("port_2", "", port_str, true, 0.5, -0.5)
    machine.plumbing:create_port("port_3", "", port_str, true, 0.5, 1.5)

    return 1, 1
end