---@module "symmetry_script"
-- Radial around a piece symmetry, uses the center (origin of the piece) 
-- and its orientation (axis configurable) as the central axis

local gui = require("gui")
local input = require("input")
local logger = require("logger")
require("editor")

local events = dofile("core:util/c_events.lua")
--local wiring = dofile("core:scenes/editor/symmetry/c_wiring.lua"):init(symmetry_mode.saved_toml)
--local plumbing = dofile("core:scenes/editor/symmetry/c_plumbing.lua"):init(symmetry_mode.saved_toml)

local center_piece_id = symmetry_mode.saved_toml:get_number_or("center_piece", -1)
local copies = symmetry_mode.saved_toml:get_number_or("copies", 3)


---@param p vehicle.piece
function on_dirty(p)
	--wiring:on_dirty(p) 
	--plumbing:on_dirty(p)
end

function on_disconnect(p)

end

local main_canvas = nil
local select_piece_canvas = nil

local function get_radius()
end

local function rebuild()
	local angle_step = 2.0 * math.pi / copies
	for angle = 0,(2.0 * math.pi),angle_step do
	end
end

local function select_piece(piece_id)
	-- return to main canvas
	symmetry_panel:set_canvas(main_canvas, true, false)	
	if piece_id ~= center_piece_id then
		center_piece_id = piece_id
		axis_offset = 0.0
		rebuild()
	end
	events:remove("select_center")
end

local function pick_piece()
	symmetry_panel:set_canvas(select_piece_canvas, false, true)
	modify_interface:start_picking_piece()
	events:add_named(edveh_interface, "on_select_piece", "select_center", select_piece)

end

local function make_main_canvas()
	local canvas = gui.canvas.new()
	local layout = gui.vertical_layout.new()

	local pick_button = gui.text_button.new("Pick center")
	events:add(pick_button, "on_clicked", function(btn) 
		if btn == input.btn.left then
			pick_piece()
		end
	end)
	
	layout:add_widget(pick_button)
	canvas:set_layout(layout)
	return canvas
end

local function make_select_piece_canvas()
	local canvas = gui.canvas.new()
	local layout = gui.vertical_layout.new()
	local label = gui.label.new("Select a piece to use as center for symmetry")
	layout:add_widget(label)
	canvas:set_layout(layout)
	return canvas
end

function take_gui_control()
	-- Note: These must be called here to sign-up for events!
	main_canvas = make_main_canvas()
	select_piece_canvas = make_select_piece_canvas()
	
	if center_piece_id < 0 then
		pick_piece()
	end
end

function leave_gui_control()
	events:remove_all()
end

function save()
	symmetry_mode.saved_toml:set_number("center_piece", center_piece_id)
	symmetry_mode.saved_toml:set_number("copies", copies)
end
