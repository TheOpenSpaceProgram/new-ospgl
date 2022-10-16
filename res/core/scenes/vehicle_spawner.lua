-- Handles spawning of a vehicle from a TOML file at a given position and orientation

local veh = require("vehicle")
local toml = require("toml")

local vehicle_spawner = {}

-- unpack defaults to true, if false, the vehicle will remain packed
-- returns the created entity
function vehicle_spawner.spawn_vehicle(universe, veh_toml, pos, vel, rot, angvel, unpack)
    if unpack == nil then unpack = true end

    local vehicle = veh.vehicle.new()
    veh_toml:read_to_vehicle(vehicle)
    vehicle:sort()

    local nent = universe:create_vehicle_entity(vehicle)
    state = {pos = pos, vel = vel, rot = rot, angvel = angvel}
    vehicle.packed:set_world_state(state)

    if unpack then
        vehicle:unpack()
    end

    return nent
end

-- unpack defaults to true, if false, vehicle will remain packed
-- TODO: and in a landed trajectory alongside the launchpad
function vehicle_spawner.spawn_vehicle_at_launchpad(universe, ent, unpack)

end

return vehicle_spawner