---@meta

---@class vehicle_editor.camera
local camera = {}

---@param w number
---@param h number
---@return renderer.camera_uniforms
function camera:get_camera_uniforms(w, h) end

---@class vehicle_editor
---@field veh vehicle
---@field edveh vehicle_editor.vehicle
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

---@class vehicle_editor.symmetry_panel
local symmetry_panel = {}

---@param canvas gui.canvas
---@param can_exit boolean Is the symmetry done being created (show done button)
---@param can_go_back boolean Can the current action be "cancelled" (show back instead of cancel)
function symmetry_panel:set_canvas(canvas, can_exit, can_go_back) end

---@class vehicle_editor.vehicle
local editor_vehicle = {}

---@param p vehicle.piece
--- Call this function after moving pieces or similar so their colliders are correctly placed
--- Updates all children pieces too!
function editor_vehicle:update_collider_hierarchy(p) end

---@param p vehicle.piece
function editor_vehicle:update_collider(p) end

---@param piece vehicle.piece
---@param to vehicle.piece
---@param attach_from string
---@param attach_to string
function editor_vehicle:attach(piece, to, attach_from, attach_to) end

---@param piece vehicle.piece
---@param highlight glm.vec3
function editor_vehicle:set_piece_highlight(piece, highlight) end

