---@meta 


---@class universe.lua_event_handler
local lua_event_handler = {}
function lua_event_handler:sign_out() end

---@class universe
---@field bt_world bullet.world
---@field system universe.planetary_system
---@field entities table Read only
local universe = {}

---@param event_id string
---@param fun function
---@return universe.lua_event_handler
function universe:sign_up_for_event(event_id, fun) end

---@param event_id string
---@param ... any further arguments
function universe:emit_event(event_id, ...) end

---@param script_path string path to the script
---@return universe.entity
function universe:creat_entity(script_path) end

---@class universe.planetary_system
local planetary_system = {}

---@class universe.entity
--- Entities are implemented in lua and work as tables!
--- (ie, you can access all public stuff in their environment)
--- but some functions are also exposed, written in C++
local entity = {}
-- TODO: Do what's said in the comment
