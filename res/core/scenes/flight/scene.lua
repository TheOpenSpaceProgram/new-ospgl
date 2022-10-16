-- The flight scene
local logger = require("logger")
local rnd = require("renderer")
require("universe")
require("game_database")
local assets = require("assets")
local glm = require("glm")
local cameras = dofile("core:scenes/cameras.lua")
local veh_spawner = dofile("core:scenes/vehicle_spawner.lua")

local renderer = osp.renderer
local universe = osp.universe

logger.info(osp)

local cubemap = assets.get_cubemap("debug_system:skybox.png")
local skybox = rnd.skybox.new(cubemap)
-- we don't need the cubemap anymore (skybox has a copy)
cubemap = nil

local sunlight = rnd.sun_light.new(osp.renderer.quality.sun_terrain_shadow_size, osp.renderer.quality.sun_shadow_size)


-- We get optionally passed the entity id of vehicle to control
function load(veh_id)
    renderer:add_drawable(universe.system)

    for _, ent in pairs(universe.entities) do
        renderer:add_drawable(ent)
    end

    universe:sign_up_for_event("core:new_entity", 
        function (id) renderer:add_drawable(universe.get_entity(id)) end)
    
    universe:sign_up_for_event("core:remove_entity", 
        function (id) renderer:remove_drawable(universe.get_entity(id)) end)

    -- Skybox and IBL generation is enabled
    renderer:add_drawable(skybox)
    renderer:add_light(sunlight)

    local veh = veh_spawner.spawn_vehicle(universe, osp.game_database:get_udata_vehicle("debug.toml"), 
        glm.vec3.new(1000000, 0, 0), glm.vec3.new(0, 0, 0), glm.quat.new(1, 0, 0, 0), glm.vec3.new(0, 0, 0), true)

end

function update(dt)
end

function render()
    renderer:render()
end

function unload()
    renderer.clear()
end

function get_camera_uniforms(width, height) 
    return cameras.from_pos_and_dir(0, 0)
end