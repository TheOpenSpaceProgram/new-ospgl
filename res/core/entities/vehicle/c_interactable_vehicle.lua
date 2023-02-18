-- Allows highlighting pieces using raycast of linked vehicle
-- Highlighted pieces can spawn dialog menus on mouse to show info, and can be interacted
-- with via all button clicks.
-- Highly customizable, created around a vehicle entity
-- Add as a drawable to the renderer
-- For an interface to support being displayed in the context menus, it must expose a
-- do_core_context_menu(self, layout: gui.vertical_layout) function

local glm = require("glm")
local input = require("input")
local bullet = require("bullet")
local model = require("model")
local guilib = require("gui")
local assets = require("assets")
local raycast = require("core:util/g_raycast.lua")
local debug_drawer = require("debug_drawer")
local logger = require("logger")

local hover_material = assets.get_material("core:mat_hover.toml")
local mat_override = model.mat_override.from_table({color = glm.vec3.new(1, 1, 1)})

---@class core.context_menu
---@field window gui.window
---@field part_id integer

---@class core.interactable_vehicle
local interactable_vehicle = {}

---@type universe.entity
interactable_vehicle.veh_ent = nil
---@type vehicle
interactable_vehicle.veh = nil

---@type integer Points to a piece, if negative means not hovering anything
interactable_vehicle.hovered = -1

--- Stores all active contextual menus
---@type table<integer, core.context_menu>
interactable_vehicle.context_menus = {}

--- Stores which parts have context menu, as only one per part is allowed
---@type table<integer, core.context_menu>
interactable_vehicle.part_context_menus = {}

---@param veh_ent universe.entity
function interactable_vehicle:init(veh_ent) 
	self.veh_ent = veh_ent
	self.veh = veh_ent.lua.vehicle
	assert(self.veh, "Linked entity was not a vehicle, this is not allowed")
	--self.veh.on_change:add_handler(self.on_dirty, self)
	self:on_veh_dirty()

	return self
end

--- Call when the vehicle entity is lost (also called on gc)
function interactable_vehicle:finish()
	self.veh = nil
	self.veh_ent = nil
end

function interactable_vehicle:on_veh_dirty()
	
end

function interactable_vehicle:update_hover(cu)
	local rstart, rend = raycast.get_mouse_ray(osp.renderer, cu, 100.0) 
	local result = osp.universe.bt_world:raycast(rstart, rend)

	local closest_hit = nil
	local closest_type = nil
	local closest_dist = math.huge
	for id, hit in pairs(result) do
		local dist = glm.length(hit.pos - rstart)
		local hit_udata = hit.rg:get_udata_type()
		debug_drawer.add_point(hit.pos, glm.vec3.new(1.0, 1.0, 0.0))
		if dist < closest_dist and (hit_udata == "piece" or hit_udata == "welded_group") then
			closest_dist = dist
			closest_type = hit_udata
			closest_hit = id
		end
	end


	if not closest_hit then
		self.hovered = -1
		return
	end

	local hit = result[closest_hit]
	local piece = nil

	-- Resolve it, pretty easy thanks to compound_id
	if closest_type == "welded_group" then
		local as_welded_group = hit.rg:get_udata_wgroup()
		assert(as_welded_group)
		piece = as_welded_group:get_piece(hit.compound_id)	
	else 
		piece = hit.rg:get_udata_piece()
	end
	assert(piece, "Could not resolve piece, something's broken in vehicle!")

	-- Make sure the piece is in our vehicle and not another
	if self.veh:get_piece_by_id(piece.id) then
		self.hovered = piece.id
	end

end

function interactable_vehicle:close(k)
	self.context_menus[k].window:close()
	local part = self.context_menus[k].part_id
	self.part_context_menus[part] = nil
	self.context_menus[k] = nil
end


---@param gui gui.screen
function interactable_vehicle:new_context_menus(gui) 
	if not input.mouse_down(input.btn.left) then return end
	if self.hovered < 0 then  
		-- A click in empty space may actually hide dialogs
		if input.key_pressed(input.key.left_control) then return end
		for k, _ in pairs(self.context_menus) do 
			self:close(k)
		end
		return
	end


	local hovered_p = self.veh:get_piece_by_id(self.hovered)
	assert(hovered_p)
	local hovered_part = hovered_p:get_part()
	-- TODO: Orphan pieces may still offer a context menu?
	if hovered_part == nil then return end
	
	if self.context_menus[hovered_p.id] then
		-- Context menu is already present, highlight it
		self.context_menus[hovered_p.id].highlight_timer = 1.0
		return
	end

	if self.part_context_menus[hovered_part.id] then
		-- Context menu is present for part, hightlight it too
		self.part_context_menus[hovered_part.id].highligh_timer = 1.0
		return
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
	n_menu.part_id = hovered_part.id
	local machine_bar, content = n_menu.window.canvas:divide_v_pixels(24 + 10)

	local top_layout = guilib.horizontal_layout.new()
	machine_bar:set_layout(top_layout)	
	
	n_menu.content = content

	---@param machine vehicle.machine
	function n_menu:build_menu(machine)
		local layout = guilib.vertical_layout.new()
		if self.active_button then
			self.active_button.toggled = false
		end
		self.buttons[machine].toggled = true

		self.active_button = self.buttons[machine]

		for name, interface in pairs(machine.interfaces) do
			if interface.do_core_context_menu then
				interface:do_core_context_menu(layout)
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
	

	-- Remove all others if CTRL is not held
	if not input.key_pressed(input.key.left_control) then
		for k, _ in pairs(self.context_menus) do 
			if k ~= self.hovered then 
				self:close(k)
			end
		end
	end

end

---@param menu core.context_menu
function interactable_vehicle:update_context_menu(p_id, menu, dt, gui) 
	if not menu.window:is_open() then
		self.context_menus[p_id] = nil
		return
	end
	
	if menu.highlight_timer > 0.0 then
		menu.highlight_timer = menu.highlight_timer - dt * 10.0
	else 
	
	end

end

---@param cu renderer.camera_uniforms Camera uniforms for the raycast (generate on physics update)
function interactable_vehicle:physics_update(cu)
	self:update_hover(cu)

end

---@param gui gui.screen
---@param cu renderer.camera_uniforms
function interactable_vehicle:draw_gui(gui, cu)
	-- Draw links
	for p_id, menu in pairs(self.context_menus) do 
		local p = self.veh:get_piece_by_id(p_id)
		assert(p)
		local p_pos_3d = p:get_graphics_position() - cu.cam_pos
		local p_pos_2d, front = glm.world_to_clip(cu.proj_view, p_pos_3d)
		if front then
			p_pos_2d = glm.clip_to_screen(p_pos_2d, gui.viewport)
			gui.skin:draw_link(p_pos_2d, menu.window.pos)
		end
	end
end

---@param gui gui.screen
function interactable_vehicle:update(dt, gui)
	-- Prevent hovering if mouse is blocked, this effectively blocks everything else
	if gui.input.mouse_blocked or gui_input.ext_mouse_blocked then
		self.hovered = -1
		return
	end

	-- Bring up context menus
	self:new_context_menus(gui)
	for p_id, menu in pairs(self.context_menus) do 
		self:update_context_menu(p_id, menu, dt, gui)
	end

end

function interactable_vehicle:forward_pass(cu, _)
	if self.hovered < 0 then return end
	local piece = self.veh:get_piece_by_id(self.hovered)
	assert(piece)
	local i = glm.inverse(piece.collider_offset) ---@cast i glm.mat4
	local tform = piece:get_graphics_transform():to_mat4() * i
	piece:get_model_node():draw_override(cu, hover_material:get(), tform, 0, mat_override, true, true)

end


return interactable_vehicle
