require("toml")
require("game_database")
require("vehicle")
local nvg = require("nano_vg")
local plumbing_lib = require("plumbing")
local assets = require("assets")
local icon = assets.get_image("machines/icons/valve.png")
function get_icon() return icon end

local display_name = osp.game_database:get_string("core:1_way_name")
function get_display_name() return display_name end

plumbing = {}

function plumbing.fluid_update()

end

function plumbing.get_connected_ports(port)
    if port == "port_2" then
        return {"port_1"}
    elseif port == "port_1" then
        return {""}
    end
end

function plumbing.get_pressure_drop(from, to, cur_P)
    return cur_P * 0.0
end

function plumbing.out_flow(port, volume, do_flow)
    return plumbing_lib.stored_fluids.new()
end

function plumbing.in_flow(port, fluids, do_flow)
    return plumbing_lib.stored_fluids.new()
end

function plumbing.get_maximum_flowrate(port)
end

function plumbing.draw_diagram(vg, skin)
    nvg.begin_path(vg)
    nvg.move_to(vg, -0.5, 0.5)
    nvg.line_to(vg, 0.0, 0.5)
    nvg.line_to(vg, 0.0, 1.0)
    nvg.line_to(vg, 0.0, 0.0)
    nvg.line_to(vg, 1.0, 1.0)
    nvg.line_to(vg, 1.0, 0.0)
    -- final line
    nvg.move_to(vg, 1.0, 0.5)
    nvg.line_to(vg, 1.5, 0.5)
    nvg.stroke(vg)

    nvg.begin_path(vg)
    -- line fill
    nvg.move_to(vg, 1.0, 1.0)
    nvg.line_to(vg, 0.7, 1.0)
    nvg.line_to(vg, 1.0, 0.7)
    nvg.line_to(vg, 1.0, 1.0)
    nvg.fill(vg)
    nvg.stroke(vg)

    -- circle
    nvg.begin_path(vg)
    nvg.circle(vg, 0.0, 0.0, 0.1)
    nvg.fill(vg)
    nvg.stroke(vg)

end

-- We create needed ports
local in_str = osp.game_database:get_string("inlet")
local out_str = osp.game_database:get_string("outlet")
function plumbing.init()
    machine.plumbing:create_port("port_1", "", out_str, true, 1.5, 0.5)
    machine.plumbing:create_port("port_2", "", in_str, true, -0.5, 0.5)
    return 1, 1
end
