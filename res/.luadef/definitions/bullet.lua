---@meta 

local bullet = {}

---@class bullet.world

---@class bullet.motion_state

---@class bullet.collision_shape

---@class bullet.transform
---@field pos glm.vec3 
---@field rot glm.quat
bullet.transform = {}
---@return glm.mat4
function bullet.transform:to_mat4() end

---@class bullet.rigidbody
bullet.rigidbody = {}
---@param mass number
---@param has_mstate boolean The motion state allows interpolation of the movement
---@param col_shape bullet.collision_shape
---@param inertia glm.vec3 If the body is non-moving, you can use (0, 0, 0)
---@return bullet.rigidbody
function bullet.rigidbody.new(mass, has_mstate, col_shape, inertia) end

---@return table Userdata table, data is stored in the rigidbody itself
function bullet.rigidbody:get_userdata() end

---@return bullet.collision_shape
function bullet.rigidbody:get_collision_shape() end
---@param mass number
---@param inertia glm.vec3
function bullet.rigidbody:set_mass_props(mass, inertia) end

---@return boolean
function bullet.rigidbody:is_in_world() end

---@param st bullet.motion_state
function bullet.rigidbody:set_motion_state(st) end

---@return bullet.motion_state
function bullet.rigidbody:get_motion_state() end

---@return glm.vec3 min
---@return glm.vec3 max
function bullet.rigidbody:get_aabb() end

---@param trans glm.vec3
function bullet.rigidbody:translate(trans) end

---@param point glm.vec3
function bullet.rigidbody:get_velocity_in_local_point(point) end

---@param vel glm.vec3
function bullet.rigidbody:set_linear_velocity(vel) end

---@param vel glm.vec3
function bullet.rigidbody:set_angular_velocity(vel) end

---@return glm.vec3
function bullet.rigidbody:get_linear_velocity() end

---@return glm.vec3
function bullet.rigidbody:get_angular_velocity() end

---@return bullet.transform
function bullet.rigidbody:get_com_transform() end

---@return glm.quat
function bullet.rigidbody:get_orientation() end

---@return glm.vec3
function bullet.rigidbody:get_com_position() end

function bullet.rigidbody:update_inertia_tensor() end

function bullet.rigidbody:clear_forces() end

---@param v glm.vec3
function bullet.rigidbody:apply_torque_impulse(v) end

---@param v glm.vec3
function bullet.rigidbody:apply_torque(v) end

---@param v glm.vec3
---@param rel_pos glm.vec3
function bullet.rigidbody:apply_force(v, rel_pos) end

---@param v glm.vec3
function bullet.rigidbody:apply_central_impulse(v) end

---@param v glm.vec3
---@param rel_pos glm.vec3
function bullet.rigidbody:apply_impulse(v, rel_pos) end

---@param v glm.vec3
function bullet.rigidbody:apply_central_force(v) end

---@param world bullet.world
function bullet.rigidbody:add_to_world(world) end

---@param world bullet.world
function bullet.rigidbody:remove_from_world(world) end

---@param mat glm.mat4
function bullet.rigidbody:set_world_transform(mat) end

---@return glm.mat4
function bullet.rigidbody:get_world_transform() end

---@return number
function bullet.rigidbody:get_friction() end
---@return number
function bullet.rigidbody:get_restitution() end

---@param val number
function bullet.rigidbody:set_friction(val) end
---@param val number
function bullet.rigidbody:set_restitution(val) end

function bullet.rigidbody:set_kinematic() end

return bullet
