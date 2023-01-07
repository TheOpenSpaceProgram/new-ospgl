---@meta 

---@class vehicle
---@field packed vehicle.packed
---@field unpacked vehicle.unpacked
---@field all_pieces vehicle.piece[]
---@field parts vehicle.part[]
---@field root vehicle.piece
local vehicle = {}

---@return vehicle
---@nodiscard
function vehicle.new() end
---@return boolean
---@nodiscard
function vehicle:is_packed() end

function vehicle:unpack() end
function vehicle:pack() end

---@param dt number
function vehicle:update(dt) end
---@param dt number
function vehicle:physics_update(dt) end

---@param universe universe
---@param entity universe.entity
function vehicle:init(universe, entity) end

function vehicle:update_attachments() end

---@param bt_world bullet.world
function vehicle:set_world(bt_world) end

---@param p vehicle.piece
function vehicle:remove_piece(p) end

function vehicle:sort() end
function vehicle:remove_outdated() end

---@param p vehicle.piece
---@return vehicle.piece[]
---@nodiscard
function vehicle:get_children_of(p) end

---@param p vehicle.piece
---@return vehicle.piece[]
---@nodiscard
function vehicle:get_attached_to(p) end


---@class vehicle.packed
---@field vehicle vehicle
local packed_vehicle = {}
---@param state table
--- state must contain:
---  -> pos (glm.vec3)
---  -> vel (glm.vec3)
---  -> rotation (glm.quat)
---  -> angular velocity (optional glm.vec3)
function packed_vehicle:set_world_state(state) end


---@class vehicle.unpacked
local unpacked_vehicle = {}

---@param renderer boolean Return center of mass used for visuals if true, otherwise CoM for physics
---@return glm.vec3
---@nodiscard
function unpacked_vehicle:get_center_of_mass(renderer) end

---@class vehicle.piece
---@field rigid_body bullet.rigidbody
---@field welded boolean
---@field attached_to vehicle.piece
---@field collider_offset glm.mat4
local piece = {}

---@param update_now boolean
function piece:set_dirty(update_now) end
---@return bullet.transform
---@nodiscard
function piece:get_global_transform() end
---@return bullet.transform
---@nodiscard
function piece:get_graphics_transform() end
---@return bullet.transform
---@nodiscard
function piece:get_local_transform() end
---@return glm.vec3
---@nodiscard
function piece:get_linear_velocity() end
---@return glm.vec3
---@nodiscard
function piece:get_angular_velocity() end
---@return glm.vec3
---@nodiscard
function piece:get_tangential_velocity() end
---@return glm.vec3
---@nodiscard
function piece:get_relative_position() end
---@return glm.vec3
---@nodiscard
function piece:get_global_position() end
---@return glm.vec3
---@nodiscard
function piece:get_graphics_position() end
---@return glm.vec3
---@nodiscard
function piece:get_forward() end
---@return glm.vec3
---@nodiscard
function piece:get_up() end
---@return glm.vec3
---@nodiscard
function piece:get_right() end

---@return glm.vec3
---@param axis glm.vec3
---@nodiscard
function piece:transform_axis(axis) end

---@param name string
---@return glm.vec3
---@nodiscard
function piece:get_marker_position(name) end

---@param name string
---@return glm.quat
---@nodiscard
function piece:get_marker_rotation(name) end

---@param name string
---@return glm.mat4
---@nodiscard
function piece:get_marker_transform(name) end

---@param name string
---@return glm.vec3
---@nodiscard
function piece:get_marker_forward(name) end

---@param p glm.vec3
---@return glm.vec3
---@nodiscard
function piece:transform_point_to_rigidbody(p) end

---@return vehicle.piece[]
---@nodiscard
function piece:get_attached_to_this() end

---@return vehicle.piece[]
---@nodiscard
function piece:get_attached_to_marker(marker) end

---@return number
---@nodiscard
function piece:get_environment_pressure() end


---@return model.node
---@nodiscard
--- DO NOT HOLD THE MODEL NODE FOR LONG, it may be invalidad and this will cause a hard crash
function piece:get_model_node() end

---@class vehicle.part
---@field id integer
local part = {}
---@return vehicle.piece
---@param name string
---@nodiscard
function part:get_piece(name) end
---@return vehicle.machine
---@param id string
---@nodiscard
function part:get_machine(id) end

---@class vehicle.machine
---@field runtime_uid integer
---@field init_toml toml.table
---@field plumbing vehicle.plumbing_machine
---@field in_part vehicle.part
local machine = {}

---@param iname string Name of the interface
---@return table
function machine:load_interface(iname) end

---@return vehicle.machine[]
---@param include_this boolean? Default to true
function machine:get_all_wired_machines(include_this) end

---@return vehicle.machine[]
--- Pass a list of interface names, and true / false for include_this (optional, default to true)
function machine:get_wired_machines_with(...) end

---@return vehicle.machine[]
---@param int_type string Interface type
---@param include_this boolean? Include this machine in list? Defaults to true
function machine:get_wired_machines(int_type, include_this) end

---@return table
---@param iname string
function machine:get_interface(iname) end

return vehicle
