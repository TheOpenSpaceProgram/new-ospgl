---@meta 


local gui = {}

---@class gui.skin
gui.skin = {}

---@param start_pos glm.vec2
---@param win_pos glm.vec2
---@param win_size glm.vec2
---@param mpos glm.vec2
---@param cutable boolean
function gui.skin:draw_link(start_pos, win_pos, win_size, mpos, cutable) end

---@param start_pos glm.vec2
---@param win_pos glm.vec2
---@param win_size glm.vec2
---@param mpos glm.vec2
---@return boolean
function gui.skin:can_cut_link(start_pos, win_pos, win_size, mpos) end

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
---@field skin gui.skin
---@field viewport glm.vec4 (x, y, w, h)
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
---@field child_pixels integer
---@field pixels_for_child_1 boolean
gui.canvas = {}

---@return gui.canvas
function gui.canvas.new() end

--- If children are set, they are removed
---@param lyout gui.layout
function gui.canvas:set_layout(lyout) end

---@param fac number How much space should the left children take? (0->1)
---@return gui.canvas Left children canvas
---@return gui.canvas Right children canvas
--- If layout is present, it's inherited by left children
function gui.canvas:divide_h(fac) end

---@param fac number How much space should the top children take? (0->1)
---@return gui.canvas Top children canvas
---@return gui.canvas Bottom children canvas
--- If layout is present, it's inherited by top children
function gui.canvas:divide_v(fac) end

---@param pixels integer How many pixels should the selected children take?
---@param for_bottom? boolean Defaults to false, are the given pixels for the top canvas or bottom one?
---@return gui.canvas Top children canvas
---@return gui.canvas Bottom children canvas
--- If layout is present, it's inherited by top children
function gui.canvas:divide_v_pixels(pixels, for_bottom) end

---@param pixels integer How many pixels should the selected children take?
---@param for_right? boolean Defaults to false, are the given pixels for the left canvas or right one?
---@return gui.canvas Left children canvas
---@return gui.canvas Right children canvas
--- If layout is present, it's inherited by left children
function gui.canvas:divide_h_pixels(pixels, for_right) end

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
---@field alpha number
---@field pos glm.vec2
---@field size glm.vec2
gui.window = {}

---@return boolean
--- You should remove any references to the window if this goes false to guarantee memory freeing
function gui.window:is_open() end

---@enum gui.window_style
gui.window_style = {
	normal = 0,
	linked = 1
}

---@class gui.widget
---@field default_size glm.vec2
---@field visible boolean Readonly
gui.widget = {}

---@class gui.base_button:gui.widget
---@field disabled boolean
---@field toggled boolean
gui.base_button = {}

---@param event_id string
---@param fun function
---@return lua_event_handler
--- Remember to store the returned event, as otherwise it will be removed and not receive events!
--- Events:
---    on_enter_hover()
---    during_hover()
---    on_leave_hover()
---	   on_clicked(btn: integer)
---	   during_click(btn: integer)
---    on_released(btn: integer)
---@nodiscard
function gui.base_button:sign_up_for_event(event_id, fun) end

---@class gui.image_button:gui.base_button
gui.image_button = {}

---@param img assets.image_handle
function gui.image_button:set_image(img) end

---@return gui.image_button
function gui.image_button.new() end

---@class gui.text_button:gui.base_button
---@field text string
---@field override_color boolean
---@field color nano_vg.color
---@field center_horizontal boolean
---@field center_vertical boolean
gui.text_button = {}

---@param text string
---@return gui.text_button
function gui.text_button.new(text) end

---@enum gui.label_style
gui.label_style = {
	title = 0,
	separator = 1
}

---@class gui.label:gui.widget
---@field text string
---@field override_color boolean
---@field color nano_vg.color
---@field center_horizontal boolean
---@field center_vertical boolean
---@field style gui.label_style
gui.label = {}

---@param text string
---@return gui.label
function gui.label.new(text) end


---@class gui.layout
---@field margins glm.vec4 Note: You may not modify it directly for implementation reasons, make a copy and then assign!
gui.layout = {}

---@param widget gui.widget
function gui.layout:add_widget(widget) end

---@param widget gui.widget
function gui.layout:remove_widget(widget) end

---@return integer
function gui.layout:get_widget_count() end

---@class gui.single_layout:gui.layout
gui.single_layout = {}

---@class gui.vertical_layout:gui.layout
gui.vertical_layout = {}

---@param elem_margin? integer
---@return gui.vertical_layout
function gui.vertical_layout.new(elem_margin) end

--- Marks next added element to be in the same-line.
function gui.vertical_layout:mark_same_line() end

---@class gui.horizontal_layout:gui.layout
gui.horizontal_layout = {}

---@param elem_margin? integer
---@return gui.horizontal_layout
function gui.horizontal_layout.new(elem_margin) end

--- Marks next added element to be in the same-line.
function gui.horizontal_layout:mark_same_line() end

---@class gui.list_layout:gui.layout
gui.list_layout = {}

return gui
