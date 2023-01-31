---@meta 


local imgui = {}

---@param name string
---@param flags? imgui.window_flags
---@param open? boolean If value is given, allows the window to be opened / closed, it will return a value then 
---@return boolean Returns false if window was closed, and true if it's still open
--- Slightly different usage than C++ ImGui
function imgui.begin(name, flags, open) end
--- Used as ImGui::End() as lua doesnt allow end keyword
function imgui.endw() end

---@param str string
function imgui.text(str) end

---@param str string
---@param r number
---@param g number
---@param b number
function imgui.text_colored(r, g, b, str) end

---@param str string
function imgui.bullet_text(str) end

function imgui.separator() end
function imgui.same_line() end

---@param str string
function imgui.tree_node(str) end

function imgui.tree_pop() end

---@param str string
---@return boolean
function imgui.button(str) end

---@param str string
---@param b boolean
---@return boolean
--- Slightly different behaviour as we don't use pointers,
--- you must use this as b = imgui.bullet_text("Wathever", b) 
--- so that b is updated!
function imgui.checkbox(str, b) end

return imgui
