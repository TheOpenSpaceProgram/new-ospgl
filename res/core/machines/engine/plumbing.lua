local plumbing = {};

local logger = require("logger")
local nvg = require("nano_vg")


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

-- Transform, colors and width are already set, just draw the lines.
-- Respect your bounding box
function plumbing.draw_diagram(vg)
    nvg.begin_path(vg)

    -- Chamber part
    nvg.move_to(vg, 0.0, 0.0)
    nvg.line_to(vg, 2.0, 0.0)
    nvg.line_to(vg, 2.0, 0.5)
    nvg.line_to(vg, 1.5, 1.0)
    nvg.line_to(vg, 1.5, 1.5)
    nvg.line_to(vg, 0.5, 1.5)
    nvg.line_to(vg, 0.5, 1.0)
    nvg.line_to(vg, 0.0, 0.5)
    nvg.line_to(vg, 0.0, 0.0)

    -- Nozzle
    nvg.move_to(vg, 0.5, 1.5)
    nvg.quad_to(vg, 0.6, 1.8, 0.75, 2.0)
    nvg.quad_to(vg, 0.6, 2.4, 0.5, 3.0)

    logger.info("DRAWING!")


end

-- Return a pair of values, width and height of the box in units
-- It will be used for dragging and preventing overlaps
function plumbing.get_draw_bounds()
    return 2
end

-- Return 4 values, first two represent position, next two direction
-- Ports will be drawn in an unified manner by the editor
function plumbing.get_port_draw_position(port)

end

return plumbing;