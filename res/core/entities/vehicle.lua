---@module 'entity_script'
-- Wraps a vehicle and converts it to an entity (ported from old VehicleEntity.cpp)
-- It's a very soft wrapper around Vehicle
local toml = require("toml")
local assets = require("assets")
local veh = require("vehicle")
local logger = require("logger")
local glm = require("glm")
require("model")
require("universe")

---@type vehicle|nil
vehicle = nil
if entity.init_toml ~= nil then
    -- vehicles are stored separately to improve the syntax of savefiles
	local vehicle_toml_path = entity.init_toml:get_string("save_vehicle")
    local vehicle_toml = assets.get_save_vehicle(vehicle_toml_path)
    vehicle = veh.vehicle.new()
    vehicle_toml:read_to_vehicle(vehicle)
end

-- Gets passed the vehicle to use
function create(nvehicle)
    vehicle = nvehicle
end

function init()
    vehicle:set_world(osp.universe.bt_world)
    vehicle:init(osp.universe, entity)
end

function update(dt)
    vehicle:update(dt)
end

function physics_update(bdt)
    vehicle:physics_update(bdt)
end

function get_input_ctx()
end

function enable_bullet(world)

end

function disable_bullet(world)
end

function deferred_pass(cu, is_env)

end

function get_visual_origin()
    if vehicle:is_packed() then
        return glm.vec3.new(0, 0, 0)
    else
        return vehicle.unpacked:get_center_of_mass(true)
    end
end

function get_physics_origin()
    if vehicle:is_packed() then
        return glm.vec3.new(0, 0, 0)
    else
        return vehicle.unpacked:get_center_of_mass(false)
    end
end

function separate_vehicle(veh)
    osp.universe:create_entity("core:entities/vehicle.lua", veh)
end

-- Rendering functions:

function needs_deferred_pass()
    return true
end

function needs_shadow_pass()
    return true
end

function deferred_pass(cu, _)
    for _, p in ipairs(vehicle.all_pieces) do
		local i = glm.inverse(p.collider_offset) ---@cast i glm.mat4
        local tform = p:get_graphics_transform():to_mat4() * i
        p:get_model_node():draw(cu, tform, entity.drawable_uid, true, false)
    end
end

function shadow_pass(cu, _)
    for _, p in ipairs(vehicle.all_pieces) do
		local i = glm.inverse(p.collider_offset) ---@cast i glm.mat4
        local tform = p:get_graphics_transform():to_mat4() * i
        p:get_model_node():draw_shadow(cu, tform, true)
    end
end
