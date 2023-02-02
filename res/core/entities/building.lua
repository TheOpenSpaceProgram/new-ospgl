-- Implements some common functionality alongside the barebones building
-- Supports the following "settings" in the building.toml file:
--  -> launchpad: [name, bounds, type]: for launchpads
--  -> interact: [script]: script gets passed ID of the building

local toml = require("toml")
local assets = require("assets")
local logger = require("logger")
local bullet = require("bullet")
local glm = require("glm")
local orbit = require("orbit")
require("model")
require("universe")

---@type assets.model_handle
local model = nil
---@type model.gpu_pointer|nil
local model_gpu = nil
---@type model.node
local draw_node = nil
---@type universe.world_state Set in update
local wstate = nil
---@type glm.mat4 Set in update
local tform = nil
---@type bullet.collision_shape
local collider = nil
---@type bullet.rigidbody|nil
local rg = nil

---@type orbit.landed_trajectory
local traj = nil

function create()

end

---@param table toml.table
local function init_from_toml(table)
	local building_file = table:get_string("building")
	logger.info("Loading building from: " .. building_file)
	local building_config = assets.get_config(building_file):get()
	-- We are going to be loading some assets, so push config pkg
	building_config:push_pkg()
	model = assets.get_model(building_config.root:get_string("model"))
	building_config:restore_pkg()

	-- Generate the collider
	local base_node = model:get():get_node("building")
	assert(base_node, "Building doesn't have 'building' node in the model")
	draw_node = base_node

	-- 'building' node must contain a collider children
	for _, child in ipairs(base_node.children) do
		if child.name:find("col_") then
			collider = child:extract_collider()
			break
		end
	end

	-- Upload the model to the GPU (TODO: Do this only when building is close to camera)
	model_gpu = model:get():get_gpu()

	local center_elem = table:get_string("in_body")
	local rel_pos = table:get_vec3("rel_pos")
	local rel_rot = table:get_quat("rel_rot")
	traj = orbit.landed_trajectory.new(center_elem, rel_pos, rel_rot)

	enable_bullet(osp.universe.bt_world)
end

function update(dt)
	wstate = traj:update(dt, false)
	tform = wstate:get_tform()
end

function needs_deferred_pass()
    return true
end

function needs_shadow_pass()
    return true
end

---@param world bullet.world
function enable_bullet(world)
	if collider then
		rg = bullet.rigidbody.new(1000000.0, false, collider, glm.vec3.new(0, 0, 0))
		rg:add_to_world(world)
	end
end

function disable_bullet(world)
	rg = nil
end

function deferred_pass(cu, _)
	if model_gpu == nil then return end
	draw_node:draw(cu, tform, 0, true, true)
end

function shadow_pass(cu, _)
	if model_gpu == nil then return end
	draw_node:draw_shadow(cu, tform, true)
end

function get_position()
	return wstate.pos
end

function get_velocity()
	return wstate.vel
end


if entity.init_toml ~= nil then init_from_toml(entity.init_toml) end
