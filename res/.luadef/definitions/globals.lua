---@meta 

---@class osp 
---@field renderer renderer
---@field audio_engine audio_engine
---@field universe universe
---@field game_database game_database
---@field game_dt number
--- Present pretty much everywhere that the engine loads. Not available on required files!
osp = {}

---@class lua_event_handler
local lua_event_handler = {}
function lua_event_handler:sign_out() end


