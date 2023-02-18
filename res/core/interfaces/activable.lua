---@module 'interface_script'
-- An activable machine can receive activation/deactivation signals. Example: Valves

local activable = create_interface()
local gui = require("gui")
local input = require("input")
local logger = require("logger")

activable.activated = false

function activable:activate()
	if not self.activated then
		self.activated = true
		self:update_button()
		if self.on_activate then
			self:on_activate()
		end
	end
end

function activable:deactivate()
	if self.deactivable and self.activated then
		self.activated = false
		self:update_button()
		if self.on_deactivate then
			self:on_deactivate()
		end
	end
end

---@param layout gui.vertical_layout
function activable:do_core_context_menu(layout)
	layout:add_widget(self.ctx_menu_button)	
end

-- (self) -> void
activable.on_activate = nil
-- (self) -> void
activable.on_deactivate = nil

activable.deactivable = true
activable.activate_string = "core:activate"
activable.deactivate_string = "core:deactivate"

--- Remember to call if you change the strings
function activable:update_button()
	if not activable.ctx_menu_button then
		activable.ctx_menu_button = gui.text_button.new("")
		activable.button_event_handler = activable.ctx_menu_button:sign_up_for_event("on_clicked", function(btn) 
			if btn == input.btn.left then
				if activable.activated then
					activable:deactivate()
				else
					activable:activate()
				end
			end
		end)

	end

	if self.activated then
		self.ctx_menu_button.text = osp.game_database:get_string(self.deactivate_string)
		if not self.deactivable then
			self.ctx_menu_button.disabled = true
		end
	else
		self.ctx_menu_button.text = osp.game_database:get_string(self.activate_string)
		self.ctx_menu_button.disabled = false
	end
end

-- Use these instead to set deactivable, this respect the GUI
function activable:set_deactivable(value)
	self.deactivable = value
	if self.activated then
		self.ctx_menu_button.disabled = true
	end
end

return activable
