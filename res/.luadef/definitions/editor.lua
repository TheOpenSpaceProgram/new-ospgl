---@meta

---@class vehicle_editor.camera
local camera = {}

---@param w number
---@param h number
---@return renderer.camera_uniforms
function camera:get_camera_uniforms(w, h) end

---@class vehicle_editor
---@field veh vehicle
---@field int vehicle_editor_interface
---@field gui_screen gui.screen
---@field cam vehicle_editor.camera
local editor = {}

---@param event_id string
---@param fun function
---@return lua_event_handler
--- Remember to store the returned event, as otherwise it will be removed and not receive events!
function editor:sign_up_for_event(event_id, fun) end

---@return glm.vec2 Screen size (to pass to editor camera to get uniforms)
---@return glm.vec4 viewport
---@return glm.vec2 real screen size (including GUI panels)
---@return glm.vec4 gui viewport
function editor:get_viewports() end

