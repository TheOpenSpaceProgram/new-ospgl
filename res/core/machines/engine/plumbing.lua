require("toml")
-- Requirements for modders:
--  You must assign the inlet marker names in the TOML given as
--  an array of marker names named "inlets". The number is determined from this.
local plumbing = {}

local inlets = {}
local logger = require("logger")
local nvg = require("nano_vg")

-- Only one (do we have it for real???)
local outlet_id = "outlet"

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

end

-- Return a pair of values, width and height of the box in units
-- It will be used for dragging and preventing overlaps
function plumbing.get_editor_size()
    return 2, 3
end

-- Return 2 values, x and y position relative to our drawing top-left
-- Ports will be drawn in an unified manner by the editor
-- Coordinates must be fractional in one of the dimensions (0.5) because
-- plumbing pipes go through the center of the grid
-- They SHOULD NEVER CHANGE as this function will be called only if ports change!
function plumbing.get_port_draw_position(port)

    if port == "outlet" then
        return 0, 0
    end

    -- Find the id number of the port
    local num = -1
    for number, name in pairs(inlet_ids) do
        if port == name then
            num = number
        end
    end

    -- This should never happen
    logger.check(num == -1)

    return positions[num]

end

-- We create needed ports
function plumbing.init()
    local inlets_toml = machine.init_toml:get_array_of_string("inlets")
    local positions = {
        {0.5, 0}, {1.5, 0}, {0, 0.5}, {2, 0.5}
    }

    for idx, inlet in pairs(inlets_toml) do
        local name = "inlet_" .. idx
        table.insert(inlets, name)
        machine.plumbing:create_port(name, inlet, unpack(positions[idx]))
    end


end

return plumbing;