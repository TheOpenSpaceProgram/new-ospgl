local glm = require("glm")
local input = require("input")
local logger = require("logger")
require("renderer")

local raycast = {}

---@param renderer renderer
---@param cu renderer.camera_uniforms
---@param dist number Maximum distance for the ray
---@return glm.vec3 Start point
---@return glm.vec3 End point
function raycast.get_mouse_ray(renderer, cu, dist)
	local mat = glm.inverse(cu.proj_view) ---@cast mat glm.mat4
	local viewport = renderer.override_viewport
	local screen_size = glm.vec2.new(renderer:get_size(true))
	local subscreen_size = screen_size * glm.vec2.new(viewport.z - viewport.x, viewport.w - viewport.y)
	local subscreen_pos = screen_size * glm.vec2.new(viewport.x, viewport.y)
	local in_subscreen = (((input.get_mouse_pos() - subscreen_pos) / subscreen_size) - glm.vec2.new(0.5)) * 2.0
	in_subscreen.y = -in_subscreen.y
	local start_loc, end_loc = glm.screen_raycast(in_subscreen, mat, dist)
	-- Now translate, this avoids multiplying by TINY numbers which errors out even with doubles
	start_loc = start_loc + cu.cam_pos
	end_loc = end_loc + cu.cam_pos
	return start_loc, end_loc
end

return raycast
