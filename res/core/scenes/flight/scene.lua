-- The flight scene
local logger = require("logger")
local rnd = require("renderer")
require("universe")
local assets = require("assets")
local glm = require("glm")
local debug_drawer = require("debug_drawer")
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

end

local function late_init()

	local pad = universe.entities[1]	
	local lpads_in_lpad = pad.lua.get_launchpads()
	local lpad = lpads_in_lpad["main"]
	local veh = veh_spawner.spawn_vehicle_at_launchpad(universe, assets.get_udata_vehicle("debug.toml"), lpad, true)
	controlled_ent = veh
	tracked_ent = veh

end

function pre_update(dt)
	osp.universe:update(dt)
end

local first_frame = true

function update(dt)
	if first_frame then
		late_init()
		first_frame = false
	end

	local ctx = tracked_ent:get_input_ctx()
	if ctx then
		ctx:update(false, dt)
	end

	---@type vehicle
	local veh = tracked_ent.lua.vehicle
	local root_tform = veh.root:get_graphics_transform():to_mat4()
	local min, max = veh:get_bounds()
	logger.info(min)
	logger.info(max)
	debug_drawer.add_box(root_tform, min, max, glm.vec3.new(1, 0, 1))
end

function render()
	renderer.env_sample_pos = tracked_ent:get_position()
	renderer:render()
end

function unload()
	renderer:clear()
end

local t = 0.0

function get_camera_uniforms(width, height)
	local offset = glm.vec3.new(math.cos(t), math.cos(t), math.sin(t)) * 40
	t = t + 0.01
	return cameras.from_pos_and_dir(tracked_ent:get_position() + offset,
		glm.vec3.new(0, 1, 0), -glm.normalize(offset), 50.0, renderer:get_size())

end
