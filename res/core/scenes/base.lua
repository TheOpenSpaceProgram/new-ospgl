--- Implements the basic ESC menu that allows saving, loading, access to config and exiting
--- different scenes. Highly customizable.

local base = {}

base.open = false

---@param gui_screen gui.screen
function base:init(gui_screen)
    self.gui_screen = gui_screen
end

function base:update()
end

return base