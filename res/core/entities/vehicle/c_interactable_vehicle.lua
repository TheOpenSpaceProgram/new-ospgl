-- Allows highlighting pieces using raycast of linked vehicle
-- Highlighted pieces can spawn dialog menus on mouse to show info, and can be interacted
-- with via all button clicks.
-- Highly customizable, created around a vehicle entity
local glm = require("glm")
local input = require("input")
local bullet = require("bullet")
local raycast = require("core:util/g_raycast.lua")
local debug_drawer = require("debug_drawer")
local logger = require("logger")

---@class core.interactable_vehicle
local interactable_vehicle = {}

---@type universe.entity
interactable_vehicle.veh_ent = nil
---@type vehicle
interactable_vehicle.veh = nil

interactable_vehicle.piece_meta = {}

---@param veh_ent universe.entity
function interactable_vehicle:init(veh_ent) 
	self.veh_ent = veh_ent
	self.veh = veh_ent.lua.vehicle
	assert(self.veh, "Linked entity was not a vehicle, this is not allowed")
	--self.veh.on_change:add_handler(self.on_dirty, self)
	self:on_veh_dirty()

	return self
end

--- Call when the vehicle entity is lost (also called on gc)
function interactable_vehicle:finish()
	self.veh = nil
	self.veh_ent = nil
end

function interactable_vehicle:on_veh_dirty()
	self.piece_meta = {}	
	
end

---@param cu renderer.camera_uniforms Camera uniforms for the raycast (generate on physics update)
function interactable_vehicle:physics_update(cu)
	local rstart, rend = raycast.get_mouse_ray(osp.renderer, cu, 1000.0) 
	debug_drawer.add_arrow(rstart, rend, glm.vec3.new(1, 0, 1))
	local result = osp.universe.bt_world:raycast(rstart, rend)

	local closest_hit = nil
	local closest_dist = math.huge
	for id, hit in pairs(result) do
		local dist = glm.length(hit.pos - rstart)
		local hit_udata = hit.rg:get_udata_type()
		if dist < closest_dist and hit_udata == "piece" or hit_udata == "welded_group" then
			closest_dist = dist
			closest_hit = id
		end
	end

end


return interactable_vehicle
