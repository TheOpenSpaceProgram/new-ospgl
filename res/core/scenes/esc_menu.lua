--- Implements the basic ESC menu that allows saving, loading, access to config and exiting
--- different scenes. Highly customizable.
local input = require("input")

local esc_menu = {}

esc_menu.open = false

---@param gui_screen gui.screen
function esc_menu:init(gui_screen)
  self.gui_screen = gui_screen
end

function esc_menu:update()
  if self.open then

  else
    local blocked = self.gui_screen.input.keyboard_blocked or self.gui_screen.input.ext_keyboard_blocked
    if input:key_down(input.key_escape) and not blocked then
      self.open = true
    end
  end
end

return esc_menu
