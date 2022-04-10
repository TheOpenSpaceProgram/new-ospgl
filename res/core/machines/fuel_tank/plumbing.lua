local plumbing = {};

require("toml")
local logger = require("logger")
local nvg = require("nano_vg")
local assets = require("assets")
require("game_database")
require("vehicle")
local plumbing_lib = require("plumbing")
require("gui")

local volume = machine.init_toml:get_number("volume")
local temperature = machine.init_toml:get_number("temperature")
local wall_mass = machine.init_toml:get_number("wall_mass")
local wall_mat = machine.init_toml:get_string("wall_material")
local wall_c = nil
if wall_mat ~= nil then wall_c = assets.get_physical_material(wall_mat).heat_capacity_solid end

local fluid_container = dofile("machines/fuel_tank/fluid_container.lua").new(volume, temperature, wall_mass, wall_c)

-- Load the contents
local content_array = machine.init_toml:get_array_of_table("content")
for _, content in ipairs(content_array) do
    local mat = content:get_string("material")
    local material = assets.get_physical_material(mat)
    local liquid_mass = content:get_number_or("liquid_mass", 0)
    local gas_mass = content:get_number_or("gas_mass", 0)
    fluid_container.contents:add_fluid(material, liquid_mass, gas_mass)
end

if fluid_container:is_overfilled() then
    logger.fatal("Tank was overfilled! Check starting fuel tank contents")
end

fluid_container:go_to_equilibrium(0.1)

-- We keep a copy just in case the machine is interested in something
plumbing.fluid_container = fluid_container

function plumbing.is_inline_element() return false end

function plumbing.fluid_update()

end

function plumbing.get_pressure(port)
    if port == "gas_port" then
        return fluid_container:get_total_pressure(0.0)
    else
        return fluid_container:get_total_pressure(10.0)
    end
end

function plumbing.out_flow(port, volume, do_flow)
    logger.info("Asked for: " .. volume * 1000 .. "L")
    return fluid_container:get_liquid(volume, do_flow)
end

function plumbing.in_flow(port, fluids, do_flow)
    -- Scale wathever we couldn't fit
    local in_volume = fluids:get_total_liquid_volume()
    logger.info("Received: " .. in_volume * 1000 .. "L")
    local final_volume = fluid_container:get_fluid_volume() + in_volume
    local accepted = fluids
    if final_volume > fluid_container.volume then
        logger.fatal("TODO: Implement rejection")
    end

    fluid_container.contents:modify(fluids)

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

function plumbing.update(dt, chemical_react)
    fluid_container:update(dt, 9.81, chemical_react)
    --fluid_container.temperature = 373
end

local liq_port_str = database:get_string("liquid_port")
local gas_port_str = database:get_string("gas_port")
function plumbing.init(ports)
    local port_marker = machine.init_toml:get_string("port_marker")
    machine.plumbing:create_port("liquid_port", port_marker, liq_port_str, false, 1.5, 3.5)
    machine.plumbing:create_port("gas_port", port_marker, gas_port_str, false, 1.5, -0.5)

    return 3, 3
end

function plumbing.get_maximum_flowrate(port)
end

return plumbing;
