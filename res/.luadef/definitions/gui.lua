---@meta 


local gui = {}

---@class gui.skin
gui.skin = {}

---@return nano_vg.color
---@param soft boolean
function gui.skin:get_foreground_color(soft) end
---@return nano_vg.color
---@param bright boolean
function gui.skin:get_background_color(bright) end
---@return gui.skin
function gui.skin.get_default_skin() end

---@class gui.screen
---@field input gui.input
gui.screen = {}

---@param skin gui.skin
---@param input gui.input
---@return gui.screen
function gui.screen.new(skin, input) end

---@param p1? glm.vec4 | integer Size to use for the gui screen / x coordinate of screen
---@param p2? integer y coordinate of screen
---@param p3? integer width of screen
---@param p4? integer height of screen
--- Call at the beginning of every frame
function gui.screen:new_frame(p1, p2, p3, p4) end

---@param canvas gui.canvas 
---@param p1 glm.vec2 | integer
---@param p2 glm.vec2 | integer
---@param p3? integer
---@param p4? integer
--- Two ways to call:
--- add_canvas(canvas, pos: vec2, size: vec2)
--- add_canvas(canvas, x, y, w, h)
function gui.screen:add_canvas(canvas, p1, p2, p3, p4) end

---@param canvas gui.canvas 
---@param p1 glm.vec2 | integer
---@param p2 glm.vec2 | integer
---@param p3? integer
---@param p4? integer
--- Adds a canvas to draw on top of everything else
--- Two ways to call:
--- add_canvas(canvas, pos: vec2, size: vec2)
--- add_canvas(canvas, x, y, w, h)
function gui.screen:add_post_canvas(canvas, p1, p2, p3, p4) end

--- Call before any input handling by 3D game so the gui can block the game
function gui.screen:prepare_pass() end

--- Call after input handling by 3D game so the gui can be blocked by the game
function gui.screen:input_pass() end

--- Draws the GUI with NanoVG. Call after prepare and input passes
function gui.screen:draw() end


---@class gui.input
---@field ext_mouse_blocked boolean Set to true to block the mouse for the GUI
---@field ext_scroll_blocked boolean Set to true to block the scroll for the GUI
---@field ext_keyboard_blocked boolean Set to true to block the keyboard for the GUI
---@field mouse_blocked boolean If true, mouse was blocked by the GUI
---@field scroll_blocked boolean If true, scroll was blocked by the GUI
---@field keyboard_blocked boolean If true, keyboard was blocked by the GUI
gui.input = {}

--- Draws a ImGUI debug screen to visualize input status. Place tactically around code to debug issues.
function gui.input:debug() end




return gui
