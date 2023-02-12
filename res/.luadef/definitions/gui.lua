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
---@field win_manager gui.win_manager
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

---@class gui.canvas
gui.canvas = {}

---@return gui.canvas
function gui.canvas.new() end

--- If children are set, they are removed
---@param lyout gui.layout
function gui.canvas:set_layout(lyout) end

---@param fac number How much space should the left children take? (0->1)
---@return gui.canvas Left children canvas
---@return gui.canvas Right children canvas
function gui.canvas:divide_h(fac) end

---@param fac number How much space should the top children take? (0->1)
---@return gui.canvas Top children canvas
---@return gui.canvas Bottom children canvas
function gui.canvas:divide_v(fac) end

---@param pixels integer How many pixels should the selected children take?
---@param for_top? boolean Defaults to true, are the given pixels for the top canvas or bottom one?
---@return gui.canvas Top children canvas
---@return gui.canvas Bottom children canvas
function gui.canvas:divide_v_pixels(pixels, for_top) end

---@param pixels integer How many pixels should the selected children take?
---@param for_left? boolean Defaults to true, are the given pixels for the left canvas or right one?
---@return gui.canvas Left children canvas
---@return gui.canvas Right children canvas
function gui.canvas:divide_h_pixels(pixels, for_left) end

---@class gui.layout
gui.layout = {}

---@class gui.win_manager
gui.win_manager = {}

---@param pos? glm.vec2 Starting position for the window, negative coordinates use default
---@param size? glm.vec2 Starting size for the window, negative coordinates use default
---@return gui.window
function gui.win_manager:create_window(pos, size) end

---@class gui.window
---@field canvas gui.canvas
---@field style gui.window_style
---@field closeable boolean
---@field pinable boolean
---@field minimizable boolean
---@field resizeable boolean
---@field moveable boolean
---@field pin_passthrough boolean
---@field title string
---@field has_titlebar boolean
gui.window = {}

---@return boolean
--- You should remove any references to the window if this goes false to guarantee memory freeing
function gui.window:is_open() end

---@enum gui.window_style
gui.window_style = {
	normal = 0,
	linked = 1
}

return gui
