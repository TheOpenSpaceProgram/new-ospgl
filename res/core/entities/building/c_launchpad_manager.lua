--- Implements launchpad functions
local launchpad_finder = require("core:entities/g_launchpad_finder.lua")
local logger = require("logger")

---@class core.launchpad
---@field in_entity integer
---@field min_bound glm.vec3
---@field max_bound glm.vec3
---@field size glm.vec3
local launchpad = {}

function launchpad:__gc()
	launchpad_finder:remove_launchpad(self.in_entity)	
end

function launchpad:get_ground()
	local ent = osp.universe.entities[self.in_entity]
	local origin_pos = ent:get_position(false)
	local origin_vel = ent:get_velocity(false)
	local origin_orient = ent:get_orientation(false)
	local origin_ang_vel = ent:get_angular_velocity(false)
	-- TODO: Transform these to the ground plane	
	return origin_pos, origin_vel, origin_orient, origin_ang_vel
end

---@return core.launchpad
function launchpad.new(entity_id, min_bound, max_bound)
	local lpad = {}
	local mtable = {}
	mtable.__index = launchpad
	setmetatable(lpad, mtable)
	lpad.in_entity = entity_id
	lpad.min_bound = min_bound
	lpad.max_bound = max_bound
	lpad.size = max_bound - min_bound
	return lpad
end

local launchpad_manager = {}

---@type core.launchpad[]
launchpad_manager.launchpads = {}

---@param table toml.table
---@param entity_id integer
---@param root_node model.node
function launchpad_manager:create_launchpad(table, entity_id, root_node)
	local marker = table:get_string("bounds")
	local min_bound, max_bound = root_node:get_bounds_of(root_node:get_child_deep(marker))
	local id = table:get_string("id")

	local n_launchpad = launchpad.new(entity_id, min_bound, max_bound)
	launchpad_finder:add_launchpad(n_launchpad.in_entity)

	logger.info(id)
	self.launchpads[id] = n_launchpad
end

---@return table<string, core.launchpad>
function launchpad_manager:get_launchpads()
	return self.launchpads 
end

return launchpad_manager
