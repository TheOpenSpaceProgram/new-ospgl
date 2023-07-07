---@module "symmetry_script"
-- Radial around a piece symmetry, uses the center (origin of the piece) 
-- and its orientation as the central axis
-- and a constant radius, which defaults to a sane value, but may be changed

local gui = require("gui")
local input = require("input")
local logger = require("logger")
local glm = require("glm")
require("vehicle")
require("editor")

local events = dofile("core:util/c_events.lua")
--local wiring = dofile("core:scenes/editor/symmetry/c_wiring.lua"):init(symmetry_mode.saved_toml)
--local plumbing = dofile("core:scenes/editor/symmetry/c_plumbing.lua"):init(symmetry_mode.saved_toml)

local center_piece_id = symmetry_mode.saved_toml:get_number_or("center_piece", -1)
local copies = symmetry_mode.saved_toml:get_number_or("copies", 3)
local radius = symmetry_mode.saved_toml:get_number_or("radius", 1.0)


---@param p vehicle.piece
function on_dirty(p)
	--wiring:on_dirty(p) 
	--plumbing:on_dirty(p)
end

function on_disconnect(p)

end

local main_canvas = nil
local select_piece_canvas = nil

---@param piece vehicle.piece
local function get_piece_radius(piece)
	-- We try to use radius metadata for given axis, if not available then we are forced to use AABBs
	-- which may not always be accurate
	local has_meta = piece.prototype.metadata:contains("radial_symmetry_radius")
	if has_meta then
		return piece.prototype.metadata:get_number("radial_symmetry_radius")
	else 
		-- TODO: AABB check
		return 1.0
	end

end

local function get_radius()
	local center = vehicle:get_piece_by_id(center_piece_id)
	assert(center)
	return get_piece_radius(center)
end

local function rebuild()
	local clones = symmetry_mode:make_clones(editor_vehicle, copies)
	local angle_step = 2.0 * math.pi / copies

	local center = vehicle:get_piece_by_id(center_piece_id)
	assert(center)

	for i, clone in ipairs(clones) do
		logger.info(clone.id)
		local angle = i * angle_step
		local offset = glm.vec3.new()
		offset.x = math.cos(angle) * radius
		offset.y = math.sin(angle) * radius
		offset.z = 0.0

		local axis = glm.rotate_from_to(glm.vec3.new(0, 0, 1), center:get_forward())
		logger.info(center:get_up())
		local axis_quat = glm.quat.new(axis)
		-- We set the piece looking at the rotation, with Z up
		-- (same behaviour as radial attachment)
		local rot = glm.quat_look_at(glm.vec3.new(0, 0, 0), offset, glm.vec3.new(0, 0, 1), glm.vec3.new(0, 1, 0))


		local tform = center:get_graphics_transform()
		local pos = axis_quat * offset + tform.pos 
		local root = symmetry_mode:get_root()
		local attach = root:get_attachment(symmetry_mode:get_attachment()).marker

		-- Attach a root piece clone at given position
		vehicle:move_piece(clone, pos, axis_quat * rot, attach)
		clone:attach_to(center, symmetry_mode:get_attachment(), "")
		clone.welded = true
		editor_vehicle:update_collider_hierarchy(clone)
	
		
	end

	vehicle:update_attachments()

	return true
end

local function select_piece(piece_id, attachment)
	-- return to main canvas
	symmetry_panel:set_canvas(main_canvas, true, false)	
	if piece_id ~= center_piece_id then
		center_piece_id = piece_id
		axis_offset = 0.0
		radius = get_radius()
		rebuild()
	end
	events:remove("select_center")
end

local function pick_piece()
	symmetry_panel:set_canvas(select_piece_canvas, false, center_piece_id > 0)
	modify_interface:start_picking_piece(false, symmetry_mode:get_all_pieces())
	events:add_named(edveh_interface, "on_select_piece", "select_center", select_piece)
end

function gui_go_back()
	symmetry_panel:set_canvas(main_canvas, true, false)	
	events:remove("select_center")
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
