local plumbing = {};

require("toml")
local logger = require("logger")
local nvg = require("nano_vg")
require("game_database")
require("vehicle")
local plumbing_lib = require("plumbing")
require("gui")

local fluid_container = dofile("machines/fuel_tank/fluid_container.lua").new(0.3)
fluid_container:go_to_equilibrium(0.1)

function plumbing.is_inline_element() return false end

function plumbing.fluid_update()

end

function plumbing.get_pressure(port)
    logger.info(fluid_container.contents)
    return fluid_container:get_total_pressure(1.0)
end

function plumbing.out_flow(port, volume, do_flow)
    return fluid_container:get_liquid(volume, do_flow)
end

function plumbing.in_flow(port, fluids, do_flow)
    return plumbing_lib.stored_fluids.new()
end

function plumbing.draw_diagram(vg, skin)
    nvg.begin_path(vg)
    nvg.rounded_rect(vg, 0, 0, 3, 3, 0.5)
    nvg.fill(vg)
    nvg.stroke(vg)

    nvg.begin_path(vg)
    nvg.move_to(vg, 1.5, 3)
    nvg.line_to(vg, 1.5, 3.5)
    nvg.move_to(vg, 1.5, 0)
    nvg.line_to(vg, 1.5, -0.5)
    nvg.stroke(vg)

    -- We draw an indicator of ullage volume
    nvg.begin_path(vg)
    nvg.rect(vg, 1.0, 0.5, 1.0, 2.0)
    nvg.stroke(vg)

    local color = skin:get_foreground_color(false)
    nvg.fill_color(vg, color)
    local fill = fluid_container:get_ullage_volume() / fluid_container.volume
    nvg.begin_path(vg)
    nvg.rect(vg, 1.0, 0.5 + fill * 2.0, 1.0, 2.0 - fill * 2.0)
    nvg.fill(vg)
end

function plumbing.update(dt)
    fluid_container:update(dt, 9.81)
    fluid_container.temperature = 373
end

local liq_port_str = database:get_string("liquid_port")
local gas_port_str = database:get_string("gas_port")
function plumbing.init(ports)
    local port_marker = machine.init_toml:get_string("port_marker")
    machine.plumbing:create_port("liquid_port", port_marker, liq_port_str, false, 1.5, 3.5)
    machine.plumbing:create_port("gas_port", port_marker, gas_port_str, false, 1.5, -0.5)

    return 3, 3
end

return plumbing;
