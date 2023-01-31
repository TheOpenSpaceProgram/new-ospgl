-- The flight scene
local logger = require("logger")
local rnd = require("renderer")
require("universe")
local assets = require("assets")
local glm = require("glm")
local cameras = dofile("core:scenes/camera_util.lua")
local veh_spawner = dofile("core:scenes/vehicle_spawner.lua")

local renderer = osp.renderer
local universe = osp.universe

local cubemap = assets.get_cubemap("debug_system:skybox.png")
local skybox = rnd.skybox.new(cubemap:move())

local sunlight = rnd.sun_light.new(osp.renderer.quality.sun_terrain_shadow_size, osp.renderer.quality.sun_shadow_size)
local envmap = rnd.envmap.new()

---@type universe.entity
local tracked_ent = nil

local event_handlers = {}

-- We get optionally passed the entity id of vehicle to control
function load(veh_id)
	renderer:add_drawable(universe.system)
	renderer:enable_env_sampling()
	for _, ent in pairs(universe.entities) do
		renderer:add_drawable(ent)
	end

	table.insert(event_handlers, universe:sign_up_for_event("core:new_entity",
		function(id) logger.info(tostring(id)) renderer:add_drawable(universe.entities[id]) end))

	table.insert(event_handlers, universe:sign_up_for_event("core:remove_entity",
		function(id) renderer:remove_drawable(universe.entities[id]) end))

	-- Skybox and IBL generation is enabled
	renderer:add_drawable(skybox)
	renderer:add_light(sunlight)
	renderer:add_light(envmap)

	--local veh = veh_spawner.spawn_vehicle(universe, assets.get_udata_vehicle("debug.toml"),
	--	glm.vec3.new(-2.720318042296709e10 + 6362e3, 1.329407956490104e10, 5.764165538717468e10),
	--	glm.vec3.new(0, 0, 0), glm.quat.new(1, 0, 0, 0), glm.vec3.new(0, 0, 0), true)
	
	local veh = veh_spawner.spawn_vehicle(universe, assets.get_udata_vehicle("debug.toml"),
		glm.vec3.new(-2.720318042296709e10 + 6465e3, 1.329407956490104e10, 5.764165538717468e10),
		glm.vec3.new(-2.975363625e4, -5.189341029e3, -2.251484e3), glm.quat.new(1, 0, 0, 0), glm.vec3.new(0, 0, 0), true)

	tracked_ent = veh

end

function pre_update(dt)
	osp.universe:update(dt)
end

function update(dt)
	local ctx = tracked_ent:get_input_ctx()
	if ctx then
		ctx:update(false, dt)
	end
end

function render()
	renderer.env_sample_pos = tracked_ent:get_visual_origin() + glm.vec3.new(10.0, 0.0, 0.0)
	renderer:render()
end

function unload()
	renderer:clear()
end

local t = 0.0

function get_camera_uniforms(width, height)
	local offset = glm.vec3.new(math.cos(t), math.cos(t), math.sin(t)) * 40
	t = t + 0.01
	return cameras.from_pos_and_dir(tracked_ent:get_visual_origin() + offset,
		glm.vec3.new(0, 1, 0), -glm.normalize(offset), 50.0, renderer:get_size())

end
