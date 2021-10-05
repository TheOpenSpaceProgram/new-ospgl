require("toml")
require("game_database")
require("vehicle")
local plumbing_lib = require("plumbing")

-- Requirements for modders:
--  You must assign the inlet marker names in the TOML given as
--  an array of marker names named "inlets". The number is determined from this.
local plumbing = {}

local inlets = {}
local logger = require("logger")
local nvg = require("nano_vg")

-- Only one (do we have it for real???)
local outlet_id = "outlet"

local inlet_count = 0

function plumbing.is_requester() return true end

function plumbing.fluid_update()

end

function plumbing.get_pressure(port)
    return 91000.0
end

function plumbing.out_flow(port, volume, do_flow)
    return plumbing_lib.stored_fluids.new()
end

function plumbing.in_flow(port, fluids, do_flow)
    logger.info(fluids)
    return plumbing_lib.stored_fluids.new()
end

function plumbing.get_free_volume(port)
    return 100.0
end

function plumbing.draw_diagram(vg, skin)

    nvg.begin_path(vg)
    -- Chamber part
    nvg.move_to(vg, 0.0, 0.0)
    nvg.line_to(vg, 2.0, 0.0)
    nvg.line_to(vg, 2.0, 0.75)
    nvg.line_to(vg, 1.5, 1.0)
    nvg.line_to(vg, 1.5, 1.5)
    nvg.line_to(vg, 0.5, 1.5)
    nvg.line_to(vg, 0.5, 1.0)
    nvg.line_to(vg, 0.0, 0.75)
    nvg.line_to(vg, 0.0, 0.0)

    -- Nozzle
    nvg.move_to(vg, 0.5, 1.5)
    nvg.quad_to(vg, 0.6, 1.8, 0.75, 2.0)
    nvg.quad_to(vg, 0.6, 2.4, 0.5, 3.0)
    nvg.line_to(vg, 1.5, 3.0)
    nvg.quad_to(vg, 2 - 0.6, 2.4, 2 - 0.75, 2.0)
    nvg.quad_to(vg, 2 - 0.6, 1.8, 2 - 0.5, 1.5)
    nvg.fill(vg)
    nvg.stroke(vg)

    nvg.begin_path(vg)

    if inlet_count >= 1 then
        nvg.move_to(vg, 0.5, 0.0)
        nvg.line_to(vg, 0.5, -0.5)
    end
    if inlet_count >= 2 then
        nvg.move_to(vg, 1.5, 0.0)
        nvg.line_to(vg, 1.5, -0.5)
    end
    if inlet_count >= 3 then
        nvg.move_to(vg, 0.0, 0.5)
        nvg.line_to(vg, -0.5, 0.5)
    end
    if inlet_count >= 4 then
        nvg.move_to(vg, 2.0, 0.5)
        nvg.line_to(vg, 2.5, 0.5)
    end

    nvg.stroke(vg)

end


local inlet_str = database:get_string("inlet")
logger.info(inlet_str)

-- We create needed ports
function plumbing.init()
    local inlets_toml = machine.init_toml:get_array_of_string("inlets")
    local positions = {
        {0.5, -0.5}, {1.5, -0.5}, {-0.5, 0.5}, {2.5, 0.5}
    }

    for idx, inlet in pairs(inlets_toml) do
        if idx > 4 then
            logger.warn("Engine was given more inlets than allowed")
            break
        end
        local name = "inlet_" .. idx
        table.insert(inlets, name)
        machine.plumbing:create_port(name, inlet, inlet_str, unpack(positions[idx]))
        inlet_count = inlet_count + 1
    end

    return 2, 3

end

return plumbing;