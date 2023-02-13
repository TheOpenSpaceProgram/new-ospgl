---@meta 

local container = {}

---@class vehicle
---@field packed vehicle.packed
---@field unpacked vehicle.unpacked
---@field all_pieces vehicle.piece[]
---@field parts vehicle.part[]
---@field root vehicle.piece
container.vehicle = {}

---@return vehicle
---@nodiscard
function container.vehicle.new() end
---@return boolean
---@nodiscard
function container.vehicle:is_packed() end

function container.vehicle:unpack() end
function container.vehicle:pack() end

---@param dt number
function container.vehicle:update(dt) end
---@param dt number
function container.vehicle:physics_update(dt) end

---@param universe universe
---@param entity universe.entity
function container.vehicle:init(universe, entity) end

function container.vehicle:update_attachments() end

---@param bt_world bullet.world
function container.vehicle:set_world(bt_world) end

---@param p vehicle.piece
function container.vehicle:remove_piece(p) end

function container.vehicle:sort() end
function container.vehicle:remove_outdated() end

---@param p vehicle.piece
---@return vehicle.piece[]
---@nodiscard
function container.vehicle:get_children_of(p) end

---@param p vehicle.piece
---@return vehicle.piece[]
---@nodiscard
function container.vehicle:get_attached_to(p) end

---@param id integer
---@return vehicle.piece|nil
function container.vehicle:get_piece_by_id(id) end

---@param id integer
---@return vehicle.part|nil
function container.vehicle:get_part_by_id(id) end

---@return glm.vec3 minimum bound (z is lower, floor of the vehicle)
---@return glm.vec3 maximum bound (z is higher, ceiling of the vehicle)
function container.vehicle:get_bounds() end

---@class vehicle.packed
---@field vehicle vehicle
 packed_vehicle = {}
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

---@return glm.vec3
---@nodiscard
function unpacked_vehicle:get_velocity() end

---@param renderer boolean Return the orientation used for visuals if true, otherwise for physics
---@return glm.quat
---@nodiscard
function unpacked_vehicle:get_orientation(renderer) end


---@class vehicle.welded_group
local welded_group = {}

--- Obtain a piece from collider id
---@param id integer
---@return vehicle.piece|nil
function welded_group:get_piece(id) end

---@class vehicle.piece
---@field rigid_body bullet.rigidbody
---@field welded boolean
---@field attached_to vehicle.piece
---@field collider_offset glm.mat4
---@field id integer Unique id of the piece, guaranteed to be unique in the vehicle
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

---@return vehicle.piece
---@nodiscard
function piece:get_attached_to_marker(marker) end

---@return number
---@nodiscard
function piece:get_environment_pressure() end


---@return model.node
---@nodiscard
--- DO NOT HOLD THE MODEL NODE FOR LONG, it may be invalidad and this will cause a hard crash
function piece:get_model_node() end

---@return vehicle.part|nil
--- If the piece is orphaned (the root piece of the part it belonged to is in another vehicle)
--- then it returns nil
--- Don't store the returned pointer for long!
function piece:get_part() end

---@class vehicle.part
---@field id integer Unique id of the part. Guaranteed to be unique in the vehicle
---@field machines table<string, vehicle.machine> Don't hold for long as it may go outdated / machines may die
local part = {}
---@return vehicle.piece
---@param name string
---@nodiscard
function part:get_piece(name) end
---@return vehicle.machine
---@param id string
---@nodiscard
function part:get_machine(id) end

---@return assets.part_prototype_handle
function part:get_prototype() end

---@class vehicle.machine
---@field runtime_uid integer
---@field init_toml toml.table
---@field plumbing vehicle.plumbing_machine
---@field in_part vehicle.part
local machine = {}

---@param iname string Name of the interface
---@return table
function machine:load_interface(iname) end

---@return string
--- Returns the asset id of the script of the machine
function machine:get_id() end

---@return boolean Was the window closed?
function machine:draw_imgui() end

---@return vehicle.machine[]
---@param include_this boolean? Default to true
function machine:get_all_wired_machines(include_this) end

---@return vehicle.machine[]
--- Pass a list of interface names, and true / false for include_this (optional, default to true)
function machine:get_wired_machines_with(...) end

---@return table[]
---@param int_type string Interface type
---@param include_this boolean? Include this machine in list? Defaults to true
function machine:get_wired_interfaces(int_type, include_this) end

---@return table
---@param iname string
function machine:get_interface(iname) end

---@class vehicle.part_prototype
---@field name string Presented ready to display, no need to localize (it's done in load time)
local part_prototype = {}

return container
