---@meta

local return_table = {}

---@enum renderer.pbr_quality
return_table.renderer_quality_pbr = {
  full = 0,
  simple_envmap = 1,
  simple = 2
}

---@class renderer.atmosphere 
---@field low_end boolean
---@field iterations number
---@field sub_iterations number

---@class renderer.pbr
---@field quality renderer.pbr_quality
---@field faces_per_sample integer
---@field frames_per_sample integer
---@field distance_per_sample number
---@field time_per_sample number
---@field planet_distance_parameter number
---@field simple_sampling boolean

---@class renderer.quality
---@field sun_shadow_size integer
---@field sun_terrain_shadow_size integer
---@field secondary_shadow_size integer
---@field env_map_size integer
---@field use_planet_detail_map boolean
---@field use_planet_detail_normal boolean
---@field pbr renderer.pbr
---@field atmosphere renderer.atmosphere

---@class renderer
---@field quality renderer.quality
---@field env_sample_pos glm.vec3
local renderer = {}

---@param draw renderer.drawable|universe.planetary_system|universe.entity
--- Warning: If you add entities to the drawable via this method, make sure you 
--- remove them once they are deleted as otherwise the game will crash!
function renderer:add_drawable(draw) end

---@param cubemap assets.cubemap_handle
--- Sets the IBL source to be a cubemap (and thus disables env sampling)
function renderer:set_ibl_source(cubemap) end

--- Disables env map sampling
function renderer:disable_env_sampling() end

--- Activates the environment mapping generator. Remember to add a "envmap" light to the renderer 
--- as otherwise it will have no effect.
function renderer:enable_env_sampling() end

---@param table table A table that conforms to the drawable standard, see documentation for this function
---@return renderer.drawable The drawable that was created from the table
--- Drawable tables:
--- They may contain the following functions, which will sign them up for the respective rendering pass:
---     - deferred_pass(camera_uniforms, is_env_map)
---     - forward_pass(camera_uniforms, is_env_map)
---     - gui_pass(camera_uniforms, is_env_map)
---     - shadow_pass(camera_uniforms, is_env_map)
---     - far_shadow_pass(camera_uniforms, is_env_map)
--- Note that by default your renderer will not be included in env_maps, to enable it
--- define drawable_env_map_enable = true inside the table
--- Upon calling this function, table.drawable_uid will be set to the assigned uid by the renderer
--- It returns the drawable so that you can remove it once it's not needed, note that unlike other 
--- drawables, removing it will destroy the returned drawable (but not the table!)
function renderer:add_table_as_drawable(table) end

---@param draw renderer.drawable|universe.planetary_system|universe.entity
function renderer:remove_drawable(draw) end

---@param light renderer.light|renderer.part_icon_light|renderer.point_light|renderer.sun_light|renderer.envmap
function renderer:add_light(light) end

---@param light renderer.light
function renderer:remove_light(light) end

function renderer:remove_all_drawables() end
function renderer:remove_all_lights() end
function renderer:render() end

--- Removes all lights and drawables
function renderer:clear() end

---@return number width
---@return number height
---@param gui boolean? Optional, if true returns gui size. Default false
function renderer:get_size(gui) end


---@class renderer.drawable
local drawable = {}

---@class renderer.camera_uniforms
---@field proj glm.mat4
---@field view glm.mat4
---@field c_model glm.mat4
---@field tform glm.mat4
---@field proj_view glm.mat4
---@field far_plane number
---@field cam_pos glm.vec3
---@field screen_size glm.vec2
local camera_uniforms = {}


---@enum renderer.light_type
return_table.light_type = {
  point = 0,
  env_map = 1,
  sun = 2,
  part_icon = 3
}

---@class renderer.light
local light = {}
---@return boolean
function light:is_added_to_renderer() end
---@return renderer.light_type
function light:get_type() end

---@class renderer.part_icon_light
---@field sun_dir glm.vec3
---@field color glm.vec3
local part_icon_light = {}
---@return renderer.light_type
function part_icon_light:get_type() end

---@class renderer.point_light
---@field pos glm.vec3
---@field color glm.vec3
---@field spec_color glm.vec3
return_table.point_light = {}
---@return renderer.light_type
function return_table.point_light:get_type() end
---@param constant number
---@param linear number
---@param quadratic number
---@param min_brightness number A value of 0.5 / 256.0 is default
function return_table.point_light:set_parameters(constant, linear, quadratic, min_brightness) end

---@return number
function return_table.point_light:get_constant() end
---@return number
function return_table.point_light:get_linear() end
---@return number
function return_table.point_light:get_quadratic() end
---@return number
function return_table.point_light:get_radius() end

---@class renderer.sun_light
---@field pos glm.vec3
---@field color glm.vec3
---@field spec_color glm.vec3
---@field ambient_color glm.vec3
return_table.sun_light = {}
---@return renderer.light_type
function return_table.sun_light:get_type() end
---@return renderer.sun_light
function return_table.sun_light.new(far_shadow_size, near_shadow_size) end

---@class renderer.envmap
return_table.envmap = {}
---@return renderer.light_type
function return_table.envmap:get_type() end
---@return renderer.envmap
function return_table.envmap.new() end

return return_table
