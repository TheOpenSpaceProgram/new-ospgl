local assets = require("assets")
local plumbing_lib = require("plumbing")
local logger = require("logger")
local nvg = require("nano_vg")
require("toml")
require("game_database")
require("vehicle")
local icon = assets.get_image("machines/icons/default_icon.png")
function get_icon() return icon end

local inlets = machine.init_toml:get_number("inlets")

plumbing = {}

function plumbing.fluid_update()

end

function plumbing.get_connected_ports(port)
    if inlets == 4 then
        if port == "port_1" then
            return {"port_2", "port_3", "port_4"}
        elseif port == "port_2" then
            return {"port_1", "port_3", "port_4"}
        elseif port == "port_3" then
            return {"port_1", "port_2", "port_4"}
        elseif port == "port_4" then
            return {"port_1", "port_2", "port_3"}
        end
    else
        if port == "port_1" then
            return {"port_2", "port_3"}
        elseif port == "port_2" then
            return {"port_1", "port_3"}
        elseif port == "port_3" then
            return {"port_1", "port_2"}
        end
    end
end

function plumbing.get_pressure_drop(from, to, cur_P)
    return 0.0
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
    -- Chamber part
    nvg.move_to(vg, 0.5, 0.5)
    nvg.line_to(vg, 1.5, 0.5)
    nvg.move_to(vg, 0.5, 0.5)
    nvg.line_to(vg, 0.5, -0.5)
    nvg.move_to(vg, 0.5, 0.5)
    nvg.line_to(vg, 0.5, 1.5)
    if inlets == 4 then
        nvg.move_to(vg, 0.5, 0.5)
        nvg.line_to(vg, -0.5, 0.5)
    end
    nvg.stroke(vg)

end

-- We create needed ports
local port_str = osp.game_database:get_string("junction_port")
function plumbing.init()
    machine.plumbing:create_port("port_1", "", port_str, true, 1.5, 0.5)
    machine.plumbing:create_port("port_2", "", port_str, true, 0.5, -0.5)
    machine.plumbing:create_port("port_3", "", port_str, true, 0.5, 1.5)
    if inlets == 4 then
        machine.plumbing:create_port("port_4", "", port_str, true, -0.5, 0.5)
    end

    return 1, 1
end
