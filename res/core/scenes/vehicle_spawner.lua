-- Handles spawning of a vehicle from a TOML file at a given position and orientation

local veh = require("vehicle")
local toml = require("toml")
local logger = require("logger")
local glm = require("glm")

local vehicle_spawner = {}

my_cool_global = "Hello, world"

-- unpack defaults to true, if false, the vehicle will remain packed
-- returns the created entity
function vehicle_spawner.spawn_vehicle(universe, veh_toml, pos, vel, rot, angvel, unpack)
	if unpack == nil then unpack = true end

	local vehicle = veh.vehicle.new()
	veh_toml:read_to_vehicle(vehicle)
	vehicle:sort()

	local nent = universe:create_entity("core:entities/vehicle/vehicle.lua", vehicle)
	state = { pos = pos, vel = vel, rot = rot, angvel = angvel }
	vehicle.packed:set_world_state(state)

	if unpack then vehicle:unpack() end
	return nent
end

---@param universe universe
---@param veh_toml toml.table
---@param lpad core.launchpad Use launchpad finder to get this
---@param unpack? boolean Defaults to true
---@param force? boolean Defauls to false, if true, the vehicle will spawn even if it doesn't fit
---@return universe.entity|nil
function vehicle_spawner.spawn_vehicle_at_launchpad(universe, veh_toml, lpad, unpack, force)
	if unpack == nil then unpack = true end
	if force == nil then force = false end

	local vehicle = veh.vehicle.new()
	veh_toml:read_to_vehicle(vehicle)
	vehicle:sort()

	local state = {}
	local pos, vel, orient, ang_vel = lpad:get_ground()
	-- Adjust position so that vehicle starts sitting in launchpad
	local min_bound, max_bound = vehicle:get_bounds()
	-- Size check
	if not force then
		local veh_size = max_bound - min_bound
		if veh_size.x > lpad.size.x or veh_size.y > lpad.size.y or veh_size.z > lpad.size.z then
			logger.warn("Vehicle doesn't fit in launchpad, cancelling spawn!")
			return
		end
	end

	-- Make it so that vehicle min_bound.z matches lpad min_bound.z
	local z_offset = lpad.min_bound.z - min_bound.z
	local up_vector = orient * glm.vec3.new(0, 0, 1)
	local off_pos = pos + z_offset * up_vector + glm.vec3.new(100.0, 100.0, 100.0)
	-- Center the vehicle otherwise in the launchpad

	state.pos = off_pos
	state.vel = vel
	state.rot = orient
	state.angvel = ang_vel

	local nent = universe:create_entity("core:entities/vehicle/vehicle.lua", vehicle)
	vehicle.packed:set_world_state(state)

	if unpack then vehicle:unpack() end
	return nent	
end

return vehicle_spawner

