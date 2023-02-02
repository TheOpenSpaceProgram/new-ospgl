---@meta 

local orbit = {}

---@class orbit.landed_trajectory
orbit.landed_trajectory = {}

---@param elem string Name of the element the object is landed on
function orbit.landed_trajectory:set_element(elem) end

---@param center_elem string Name of the element the object is landed on
---@param initial_pos glm.vec3 Position at t=0 and without rotation at epoch
---@param initial_rotation glm.quat Rotation at t=0 and without rotation at epoch
---@return orbit.landed_trajectory
function orbit.landed_trajectory.new(center_elem, initial_pos, initial_rotation) end

---@param dt number
---@param bullet boolean
---@return universe.world_state
function orbit.landed_trajectory:update(dt, bullet) end

return orbit
