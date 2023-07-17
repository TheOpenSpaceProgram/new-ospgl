---@module "editor_script"
-- Added as an editor script for quick visualization of symmetry modes

local imgui = require("imgui")
local logger = require("logger")
require("vehicle")

local events = dofile("util/c_events.lua")

local last_num = -1
local highlighted = -1

local function check_changed() 
	local sym_groups = editor.veh.meta.symmetry_groups
	if #sym_groups ~= last_num then
		highlighted = -1
	end
	last_num = #sym_groups
end

local function post_editor_update(_)
	check_changed()
	
	if highlighted ~= -1 then
		logger.info("HLIGHT!")
	end
end

function do_imgui_debug()
	local sym_groups = editor.veh.meta.symmetry_groups
	check_changed()

	if highlighted == -1 then
		for k, sym_group in pairs(sym_groups) do	
			if imgui.button("View symmetry group " .. k) then
				highlighted = k
			end
		end
	else
		imgui.text("Viewing symmetry group " .. highlighted)
		if imgui.button("Stop viewing") then
			highlighted = -1
		end
	end
end

events:add(editor, "post_editor_update", post_editor_update)

