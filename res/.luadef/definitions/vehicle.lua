---@meta 

---@class symmetry_mode
---@field saved_toml toml.table Modify this table to save data, at start it contains serialized data
local symmetry_mode_type = {}
---@param count integer Number of clones to make, including the already existing piece
---@return vehicle.piece[] Clones including the already existing piece
--- Remember to call editor_vehicle:update_collider_hierarchy(clone) after you have positioned the clones!
function symmetry_mode_type:make_clones(count) end

---@return vehicle.piece
function symmetry_mode_type:get_root() end

---@return string Name of the attachment marker
function symmetry_mode_type:get_attachment() end

---@return vehicle.piece[]
function symmetry_mode_type:get_all_pieces() end

---@return vehicle.piece[]
--- Includes root!
function symmetry_mode_type:get_root_clones() end




---@class vehicle_meta
---@field symmetry_groups symmetry_mode[]
local vehicle_meta = {}

---@param machine vehicle.machine
function vehicle_meta:set_controlled_machine(machine) end

---@return flight_input.context|nil
function vehicle_meta:get_input_ctx() end

local container = {}

---@class vehicle
---@field packed vehicle.packed
---@field unpacked vehicle.unpacked
---@field all_pieces vehicle.piece[]
---@field parts vehicle.part[]
---@field root vehicle.piece
---@field meta vehicle_meta
container.vehicle = {}

---@param event_id string
---@param fun function
---@return lua_event_handler
--- Remember to store the returned event handler, as otherwise it will be removed and not receive events!
function container.vehicle:sign_up_for_event(event_id, fun) end

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

---@param piece vehicle.piece
---@param pos glm.vec3
---@param rot glm.quat
---@param marker string Marker to use as origin, set to "" for none 
function container.vehicle:move_piece(piece, pos, rot, marker) end

---@param id string
---@return vehicle.logical_group|nil
function container.vehicle:get_logical_group(id) end

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

---@class vehicle.attachment
---@field hidden boolean
---@field marker string
---@field name string (you must use game_database to get display string)
---@field radial boolean 
---@field stack boolean 
---@field size number 
local attachment = {}

---@class vehicle.piece
---@field rigid_body bullet.rigidbody
---@field welded boolean
---@field attached_to vehicle.piece
---@field collider_offset glm.mat4
---@field prototype vehicle.piece_prototype
---@field id integer Unique id of the piece, guaranteed to be unique in the vehicle
local piece = {}

---@param name string Name for the attachment
---@return vehicle.attachment the attachment itself
---@return boolean is the attachment used?
--- Note: Returns a non-modificable copy of the attachment!
function piece:get_attachment(name) end

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

---@param target vehicle.piece
---@param attachment_marker string Name of the attachment point to use
---@param target_marker string Name of attachment point to use in target part, or "" if none
--- Note: You must create links if such are needed, or set welded to true!
--- IMPORTANT: After all attachments are done, call vehicle:update_attachments()
function piece:attach_to(target, attachment_marker, target_marker) end

--- Dettaches the piece from wathever it's attached to, correctly updating attachments and links
--- IMPORTANT: After all detachments are done, call vehicle:update_attachments()
function piece:detach() end

---@class vehicle.part
---@field id integer Unique id of the part. Guaranteed to be unique in the vehicle
---@field machines table<string, vehicle.machine> Don't hold for long as it may go outdated / machines may die. Doesn't include attached machines
---@field all_machines table<string, vehicle.machine> Don't hold for long as it may go outdated / machines may die.
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
---@field in_part_id string
---@field interfaces table<string, table>
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

---@return assets.image_handle
function machine:get_icon() end

---@return string
function machine:get_display_name() end

---@return flight_input.context|nil
function machine:get_input_ctx() end

---@param ctx flight_input.context
function machine:set_input_ctx(ctx) end

---@class vehicle.part_prototype
---@field name string Presented ready to display, no need to localize (it's done in load time)
local part_prototype = {}

---@class vehicle.piece_prototype
---@field metadata toml.table
local piece_prototype = {}

---@class vehicle.logical_group
local logical_group = {}

---@return vehicle.machine[]
---@param to vehicle.machine Machine whose connections we are interested in querying
---@param include_this boolean? Include this machine in list? Defaults to true
function logical_group:get_all_connected(to, include_this) end

---@return vehicle.machine[]
---@param to vehicle.machine Machine whose connections we are interested in querying
--- Furthermore, pass a list of interface names, and true / false for include_this (optional, default to true)
function logical_group:get_connected_with(to, ...) end

---@return table[]
--- TODO: Make a lua language server plugin so this returns the correct interface prototype file!
---@param to vehicle.machine Machine whose connections we are interested in querying
---@param int_type string Interface type
---@param include_this boolean? Include this machine in list? Defaults to true
function logical_group:get_connected_interfaces(to, int_type, include_this) end

return container
