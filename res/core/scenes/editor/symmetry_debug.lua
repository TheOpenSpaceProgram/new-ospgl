---@module "editor_script"
-- Added as an editor script for quick visualization of symmetry modes

local imgui = require("imgui")
local logger = require("logger")
local glm = require("glm")
require("vehicle")

events = dofile("util/c_events.lua")

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
		local sym = editor.veh.meta.symmetry_groups[highlighted]

		local all_pieces = sym:get_all_pieces()
		for _, p in pairs(all_pieces) do
			editor.edveh:set_piece_highlight(p, glm.vec3.new(0, 1, 1))
		end

		local root_clones = sym:get_root_clones()
		for _, p in pairs(root_clones) do
			editor.edveh:set_piece_highlight(p, glm.vec3.new(0, 1, 0))
		end

		local root = sym:get_root()
		editor.edveh:set_piece_highlight(root, glm.vec3.new(1, 0, 0))
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
		imgui.text_colored(1, 0, 0, "Root piece")
		imgui.text_colored(0, 1, 0, "Cloned root piece")
		imgui.text_colored(0, 1, 1, "Piece belongs to symmetry group")
		if imgui.button("Stop viewing") then
			highlighted = -1
		end
	end
end

events:add(editor, "post_editor_update", post_editor_update)

