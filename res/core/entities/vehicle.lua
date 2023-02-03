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


---@type vehicle
vehicle = nil
local veh_debug = nil 


-- Gets passed the vehicle to use
---@param nvehicle vehicle
function create(nvehicle)
    vehicle = nvehicle
	veh_debug = dofile("core:entities/vehicle_debug.lua"):init(vehicle)
end

function init()
	if entity.init_toml ~= nil then
    	-- vehicles are stored separately to improve the syntax of savefiles
		local vehicle_toml_path = entity.init_toml:get_string("save_vehicle")
    	local vehicle_toml = assets.get_save_vehicle(vehicle_toml_path)
    	vehicle = veh.vehicle.new()
    	vehicle_toml:read_to_vehicle(vehicle)
		veh_debug = dofile("core:entities/vehicle_debug.lua"):init(vehicle)
	end

    vehicle:set_world(osp.universe.bt_world)
    vehicle:init(osp.universe, entity)
end

function update(dt)
    vehicle:update(dt)
	veh_debug:draw_machines()
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

function get_position(physics)
    if vehicle:is_packed() then
        return glm.vec3.new(0, 0, 0)
    else
        return vehicle.unpacked:get_center_of_mass(not physics)
    end
end

function get_velocity()
	if vehicle:is_packed() then 
		return glm.vec3.new(0, 0, 0)
	else 
		return vehicle.unpacked:get_velocity()
	end
end

function separate_vehicle(veh)
    osp.universe:create_entity("core:entities/vehicle.lua", veh)
end



function do_debug_imgui()
	veh_debug:draw_main()
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
