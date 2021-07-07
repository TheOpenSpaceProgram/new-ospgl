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

function plumbing.draw_diagram(vg)

    nvg.begin_path(vg)
    nvg.rounded_rect(vg, 0, 0, 1, 3, 0.5)
    nvg.fill(vg)
    nvg.stroke(vg)

end

-- Return a pair of values, width and height of the box in units
-- It will be used for dragging and preventing overlaps
function plumbing.get_editor_size()
    return 1, 3
end

function plumbing.get_port_draw_position(port)

end

function plumbing.init(ports)

end

return plumbing;
