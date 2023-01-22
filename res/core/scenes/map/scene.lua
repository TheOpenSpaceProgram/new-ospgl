---@module 'scene_script'
-- The map scene shows an overview of the solar system, including orbits and orbit predictions
-- Highly configurable so it can be used in many places

local rnd = require("renderer")
require("universe")
local glm = require("glm")
local gui = require("gui")
local assets = require("assets")
require("toml")

local gui_screen = gui.screen.new(gui.skin.get_default_skin(), gui_input)
local universe = osp.universe
local renderer = osp.renderer
local anim = nil
local config = nil


local cubemap = assets.get_cubemap("debug_system:skybox.png")
local sunlight = rnd.sun_light.new(osp.renderer.quality.sun_terrain_shadow_size, osp.renderer.quality.sun_shadow_size)
local skybox = rnd.skybox.new(cubemap:move())
local camera = dofile("core:scenes/map/map_camera.lua"):init(universe)

---@param animation core.map.anim
---@param map_id string
---@param nconfig table
function load(animation, map_id, nconfig)
  if map_id == nil then map_id = "default" end
  -- Save the anim so it can be used on close too
  anim = animation
  config = nconfig
  -- Load map camera if saved for same map_id, or start at the sun
  local sets = universe.save_db:get_toml("map")
  if sets:contains("center_entity") then

  else

  end
  camera.sensitivity = sets:get_number_or("cam_sensitivity", 0.1)

  -- We only draw the universe and markers
  renderer:add_drawable(universe.system)
  renderer:add_drawable(skybox)
  renderer:add_light(sunlight)

end

function update(dt)
  gui_screen:new_frame()
  gui_screen:prepare_pass()
  
  camera:update()

  gui_screen:input_pass()
  gui_screen:draw()
end

function pre_update(dt)
  osp.universe:update(dt)
end

function render()
  renderer:render()
end

function unload()
  renderer:clear()
end

function close_map() 

end

function get_camera_uniforms(width, height) 
  return camera:get_camera_uniforms(width, height)
end
