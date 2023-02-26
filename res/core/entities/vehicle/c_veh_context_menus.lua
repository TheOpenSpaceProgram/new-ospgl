local glm = require("glm")
local input = require("input")
require("bullet")
local model = require("model")
local guilib = require("gui")
local assets = require("assets")
local raycast = require("core:util/g_raycast.lua")
local debug_drawer = require("debug_drawer")
local logger = require("logger")
require("game_database")


---@class core.context_menu
---@field window gui.window
---@field part_id integer
---@field last_pos_2d glm.vec2|nil
---@field was_link_blocked boolean
---@field piece_id integer

local menus = {}

-- Interfaces to be activated manually via the context menu
menus.has_interfaces = true
-- On machines which offer input contexts, allows taking control of them
menus.has_input_contexts = true
-- Tweakables to be exposed in the editor
menus.has_editor = false

---@type function|nil
--- Takes the machine as only argument
menus.on_take_control = nil

--- Stores all active contextual menus
---@type table<integer, core.context_menu>
menus.context_menus = {}

--- Stores which parts have context menu, as only one per part is allowed
---@type table<integer, core.context_menu>
menus.part_context_menus = {}

--- Stores active cut menus
---@type core.context_menu[]
menus.free_menus = {}


function menus:on_lost_piece(id, veh)
	local piece = veh:get_piece_by_id(id)
	assert(piece)
	if self.context_menus[id] then
		self:close(id)
	end
	-- It may also be a free-standing menu
	-- We iterate in reverse to prevent skipping elements
	for i = #self.free_menus,1,-1 do
		local menu = self.free_menus[i]
		if menu.piece_id == id then
			menu.window:close()
			table.remove(self.free_menus, i)
		end
	end
end

function menus:close(k)
	self.context_menus[k].window:close()
	local part = self.context_menus[k].part_id
	self.part_context_menus[part] = nil
	self.context_menus[k] = nil
end

logger.info("HERE")
local take_control_str = osp.game_database:get_string("core:take_control")
local controlled_str = osp.game_database:get_string("core:controlled")

---@param gui gui.screen
---@param veh_ent vehicle
function menus:handle_new_menu(hovered_p, gui, veh)
	local hovered_part = hovered_p:get_part()
	-- TODO: Orphan pieces may still offer a context menu?
	if hovered_part == nil then return nil end
	
	if self.context_menus[hovered_p.id] then
		-- Context menu is already present, highlight it
		self.context_menus[hovered_p.id].highlight_timer = 1.0
		return self.context_menus[hovered_p.id]
	end

	if self.part_context_menus[hovered_part.id] then
		-- Context menu is present for part, hightlight it too
		self.part_context_menus[hovered_part.id].highligh_timer = 1.0
		return self.part_context_menus[hovered_part.id]
	end

	local n_menu = {}
	n_menu.highlight_timer = 1.0
	local pos = input.get_mouse_pos()
	-- TODO: Find appropiate free location to spawn context menu
	pos = pos + glm.vec2.new(90.0, 25.0)
	size = glm.vec2.new(300.0, 300.0)
	n_menu.window = gui.win_manager:create_window(pos, size)
	-- By default the window is a "tooltip" like, if you cut the link it becomes proper
	n_menu.window.style = guilib.window_style.linked
	local proto = hovered_part:get_prototype()
	n_menu.window.title = proto:get().name
	n_menu.window.minimizable = false
	n_menu.window.closeable = false
	n_menu.window.pinable = false
	n_menu.window.alpha = 0.5
	n_menu.piece_id = hovered_p.id
	n_menu.part_id = hovered_part.id
	local machine_bar, content = n_menu.window.canvas:divide_v_pixels(24 + 10)

	local top_layout = guilib.horizontal_layout.new()
	machine_bar:set_layout(top_layout)	
	
	n_menu.content = content
	n_menu.menus = self

	---@param machine vehicle.machine
	function n_menu:build_menu(machine)
		local layout = guilib.vertical_layout.new()
		-- We achieve a more compact look by using this
		local m = layout.margins
		m.z = m.z - 8
		layout.margins = m
		
		if self.active_button then
			self.active_button.toggled = false
		end
		self.buttons[machine].toggled = true

		self.active_button = self.buttons[machine]

		-- Add title
		local title = guilib.label.new(machine:get_display_name())
		title.style = guilib.label_style.separator
		layout:add_widget(title)

		if self.menus.has_input_contexts then
			local ctx = machine:get_input_ctx()
			if ctx then
				local btn = guilib.text_button.new(take_control_str)
				local current = veh.meta:get_input_ctx()
				if current and current:is_same_as(ctx) then
					btn.text = controlled_str
					btn.disabled = true
				end
				n_menu.handlers:add(btn, "on_clicked", function(b) 
					if b == input.btn.left and self.menus.on_take_control then
						self.menus.on_take_control(machine)
						btn.text = controlled_str
						btn.disabled = true
					end
				end)
				layout:add_widget(btn)
			end
		end

		if self.menus.has_interfaces then
			for name, interface in pairs(machine.interfaces) do
				if interface.do_core_context_menu then
					interface:do_core_context_menu(layout)
				end
			end
		end

		self.content:set_layout(layout)
	end
	
	n_menu.buttons = {}
	n_menu.handlers = dofile("core:util/c_events.lua")
	local first_machine = nil
	for _, machine in pairs(hovered_part.all_machines) do
		if not first_machine then first_machine = machine end
		-- We may store the machine freely
		local button = guilib.image_button.new()
		button.default_size = glm.vec2.new(24, 24)
		button:set_image(machine:get_icon())
		n_menu.handlers:add(button, "on_clicked", function (btn)
			if btn == input.btn.left then
				n_menu:build_menu(machine)
			end
		end)
		top_layout:add_widget(button)
		--top_layout:mark_same_line()

		n_menu.buttons[machine] = button
	end
			
	n_menu:build_menu(first_machine)

	self.context_menus[hovered_p.id] = n_menu
	self.part_context_menus[hovered_part.id] = n_menu

	return n_menu
end

---@param menu core.context_menu
---@param gui gui.screen
function menus:update_menu(p_id, menu, dt, gui, veh) 
	if not menu.window:is_open() then
		self:close(p_id)
		return
	end
	
	if menu.highlight_timer > 0.0 then
		menu.highlight_timer = menu.highlight_timer - dt * 10.0
	else 

	-- Link cutting
	if menu.last_pos_2d then
		local can_cut = gui.skin:can_cut_link(menu.last_pos_2d, menu.window.pos, 
				menu.window.size, input.get_mouse_pos())
		local blocked = gui.input.mouse_blocked or gui.input.ext_keyboard_blocked
		menu.was_link_blocked = blocked
		if can_cut and not blocked then
			if input.mouse_down(input.btn.left) then	
				menu.window.style = guilib.window_style.normal
				menu.window.closeable = true
				menu.window.pinable = true
				menu.window.minimizable = true
				-- Move to free standing menus
				local hovered_part = veh:get_piece_by_id(p_id):get_part()
				assert(hovered_part)
				self.part_context_menus[hovered_part.id] = nil
				self.context_menus[p_id] = nil
				table.insert(self.free_menus, menu)
			end
			gui.input.ext_mouse_blocked = true
		end
	end
	
	end

end

---@param gui gui.screen
---@param cu renderer.camera_uniforms
function menus:draw_gui(gui, viewport, cu, veh)
	-- Draw links
	for p_id, menu in pairs(self.context_menus) do 
		local p = veh:get_piece_by_id(p_id)
		assert(p)
		local p_pos_3d = p:get_graphics_position() - cu.cam_pos
		local p_pos_2d, front = glm.world_to_clip(cu.proj_view, p_pos_3d)
		if front then
			p_pos_2d = glm.clip_to_screen(p_pos_2d, viewport)
			menu.last_pos_2d = p_pos_2d
			gui.skin:draw_link(p_pos_2d, menu.window.pos, menu.window.size, input.get_mouse_pos(), not menu.was_link_blocked)
		end
	end
end

function menus:update(dt, gui, veh)
	for p_id, menu in pairs(self.context_menus) do 
		self:update_menu(p_id, menu, dt, gui, veh)
	end
end

function menus:close_all()
	for k, _ in pairs(self.context_menus) do
		self:close(k)
	end
end

return menus
