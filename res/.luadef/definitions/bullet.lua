---@meta 


---@class transform
---@field pos vec3 
---@field rot quat
local transform = {}
---@return mat4
function transform:to_mat4() end

---@class rigidbody
local rigidbody = {}
---@param mass number
---@param mstate motion_state
---@param col_shape collision_shape
---@param inertia vec3
---@return rigidbody
function rigidbody.new(mass, mstate, col_shape, inertia) end

---@return collision_shape
function rigidbody:get_collision_shape() end
---@param mass number
---@param inertia vec3
function rigidbody:set_mass_props(mass, inertia) end

