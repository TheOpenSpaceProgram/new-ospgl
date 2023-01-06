---@meta 

local bullet = {}

---@class bullet.transform
---@field pos glm.vec3 
---@field rot glm.quat
bullet.transform = {}
---@return glm.mat4
function bullet.transform:to_mat4() end

---@class bullet.rigidbody
bullet.rigidbody = {}
---@param mass number
---@param mstate bullet.motion_state
---@param col_shape bullet.collision_shape
---@param inertia glm.vec3
---@return bullet.rigidbody
function bullet.rigidbody.new(mass, mstate, col_shape, inertia) end

---@return bullet.collision_shape
function bullet.rigidbody:get_collision_shape() end
---@param mass number
---@param inertia glm.vec3
function bullet.rigidbody:set_mass_props(mass, inertia) end

return bullet
