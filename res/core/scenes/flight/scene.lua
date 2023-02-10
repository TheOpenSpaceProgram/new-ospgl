-- The flight scene
local logger = require("logger")
local rnd = require("renderer")
local gui = require("gui")
require("universe")
local assets = require("assets")
local debug_drawer = require("debug_drawer")
local glm = require("glm")
local veh_spawner = require("core:scenes/vehicle_spawner.lua")

local renderer = osp.renderer
local universe = osp.universe

local cubemap = assets.get_cubemap("debug_system:skybox.png")
local skybox = rnd.skybox.new(cubemap:move())
local gui_screen = gui.screen.new(gui.skin.get_default_skin(), gui_input)
local sunlight = rnd.sun_light.new(osp.renderer.quality.sun_terrain_shadow_size, osp.renderer.quality.sun_shadow_size)
sunlight.track_star = true
local envmap = rnd.envmap.new()

local camera = dofile("core:scenes/flight/flight_camera.lua"):init(universe, gui_input)

---@type universe.entity
local tracked_ent = nil
local interactable_veh = dofile("core:entities/vehicle/c_interactable_vehicle.lua")

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
	renderer:add_table_as_drawable(interactable_veh)
	renderer:add_light(sunlight)
	renderer:add_light(envmap)

end

local function late_init()

	local pad = universe.entities[1]	
	local lpads_in_lpad = pad.lua.get_launchpads()
	local lpad = lpads_in_lpad["main"]
	local veh = veh_spawner.spawn_vehicle_at_launchpad(universe, assets.get_udata_vehicle("debug.toml"), lpad, true)
	assert(veh)

	controlled_ent = veh
	tracked_ent = veh
	camera.tracked_veh = veh

	interactable_veh:init(veh)

end

function pre_update(dt)
	osp.universe:update(dt)
end

local first_frame = true
local lwidth, lheight = 1, 1

local raycast = require("core:util/g_raycast.lua")

function update(dt)
	if first_frame then
		late_init()
		first_frame = false
	end

	gui_screen:new_frame()
	gui_screen:prepare_pass()

	local block = camera:update(dt)
	gui_input.ext_mouse_blocked = gui_input.ext_mouse_blocked or block
	local ent_blocked_kb = false
	if controlled_ent then
		local input_ctx = controlled_ent:get_input_ctx()
		if input_ctx then
			ent_blocked_kb = input_ctx:update(gui_input.keyboard_blocked, dt)
		end
	end
	gui_input.ext_keyboard_blocked = gui_input.ext_keyboard_blocked or ent_blocked_kb
	
	interactable_veh:update(dt, gui_screen)


	gui_screen:input_pass()
	gui_screen:draw()

end


function physics_update(dt)
	camera.physics = true
	local cu = camera:get_camera_uniforms(lwidth, lheight)
	interactable_veh:physics_update(cu)
	camera.physics = false
end
	

function render()
	if tracked_ent then
		renderer.env_sample_pos = tracked_ent:get_position(false)
	end
	renderer:render()
end

function unload()
	renderer:clear()
end

function get_camera_uniforms(width, height)
	lwidth = width
	lheight = height
	return camera:get_camera_uniforms(width, height)
end
