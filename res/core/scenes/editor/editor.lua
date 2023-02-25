---@module "editor_script"
-- This is technically not a scene, but simply a script called from the editor
-- to allow customizing certain behaviour
-- A global, "editor" of type editor_scene is passed

require("editor")
local input = require("input")
local logger = require("logger")

-- Must be global to prevent gargabe collection of the whole file!
events = dofile("util/c_events.lua")

local ctx = dofile("entities/vehicle/c_veh_context_menus.lua")
ctx.has_interfaces = false
ctx.has_editor = true

local function on_alt_click(piece_id)
	local ctrl_down = input.key_pressed(input.key.left_control)
	if piece_id < 0 then
		if not ctrl_down then
			ctx:close_all()	
		end
	else
		local hovered_p = editor.veh:get_piece_by_id(piece_id)
		local nmenu = ctx:handle_new_menu(hovered_p, editor.gui_screen, editor.veh)
		if not ctrl_down then
			for k, menu in pairs(ctx.context_menus) do 
				if menu ~= nmenu then
					ctx:close(k)
				end
			end
		end
	end
end

local function on_lost_piece(id)

end

local dt = 0

local function update(ndt)
	dt = ndt
end

local function on_gui_input()
	ctx:update(dt, editor.gui_screen, editor.veh)
end

local function post_gui_draw()
	local r, _, _, vport = editor:get_viewports()
	local cu = editor.cam:get_camera_uniforms(r.x, r.y)
	ctx:draw_gui(editor.gui_screen, vport, cu, editor.veh)
end

events:add(editor.veh, "on_lost_piece", on_lost_piece)
events:add(editor.int, "on_alt_click", on_alt_click)
events:add(editor, "on_gui_input", on_gui_input)
events:add(editor, "on_editor_update", update)
events:add(editor, "post_gui_draw", post_gui_draw)
