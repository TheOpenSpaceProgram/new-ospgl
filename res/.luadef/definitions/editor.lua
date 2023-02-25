---@meta

---@class vehicle_editor
---@field veh vehicle
local editor = {}

---@param event_id string
---@param fun function
---@return lua_event_handler
--- Remember to store the returned event, as otherwise it will be removed and not receive events!
function editor:sign_up_for_event(event_id, fun) end

