-- On / off valve, nothing fancy, operated by external "electrical" event (wiring)
require("toml")
require("game_database")
require("vehicle")
local nvg = require("nano_vg")
local plumbing_lib = require("plumbing")
local assets = require("assets")
local icon = assets.get_image("machines/icons/valve.png")
local imgui = require("imgui")
function get_icon() return icon end

plumbing = {}

local open = false
local force_open = false
local force_close = false

function draw_imgui()
	local status_str = "Status: "
	if open then
		status_str = status_str .. "Open"
	else
		status_str = status_str .. "Closed"
	end
	if force_close or force_open then
		status_str = status_str .. " (Forced)"
	end

    imgui.text(status_str)

    force_open = imgui.checkbox("Force Open", force_open)
    force_close = imgui.checkbox("Force Close", force_close)

    if force_open and force_close then
        force_close = false
    end

    if force_open then
        open = true
    elseif force_close then
        open = false
    end
end

function plumbing.fluid_update()

end

function plumbing.get_connected_ports(port)
    if open then
        if port == "port_2" then
            return {"port_1"}
        elseif port == "port_1" then
            return {"port_2"}
        end
    else
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

    -- Line
    nvg.begin_path(vg)
    nvg.move_to(vg, -0.5, 0.5)
    nvg.line_to(vg, 1.5, 0.5)
    nvg.stroke(vg)

    -- Two triangles
    nvg.begin_path(vg)
    nvg.move_to(vg, 0.0, 0.0)
    nvg.line_to(vg, 0.0, 1.0)
    nvg.line_to(vg, 0.5, 0.5)
    nvg.line_to(vg, 0.0, 0.0)
    nvg.fill(vg)
    nvg.stroke(vg)
    nvg.begin_path(vg)
    nvg.move_to(vg, 1.0, 0.0)
    nvg.line_to(vg, 1.0, 1.0)
    nvg.line_to(vg, 0.5, 0.5)
    nvg.line_to(vg, 1.0, 0.0)
    nvg.fill(vg)
    nvg.stroke(vg)

    -- Ball
    nvg.begin_path(vg)
    nvg.circle(vg, 0.5, 0.5, 0.25)
    nvg.fill(vg)
    nvg.stroke(vg)

    -- Open / close line
    nvg.begin_path(vg)
    if open then
		nvg.move_to(vg, 0.0, 0.5)
		nvg.line_to(vg, 1.0, 0.5)
	else
        nvg.move_to(vg, 0.5, 0.0)
        nvg.line_to(vg, 0.5, 1.0)
	end

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

-- Implementation of activable interface
local activable = machine:load_interface("core:interfaces/activable.lua")
activable.on_activate = function () open = true end 
activable.on_deactivate = function() open = false end
