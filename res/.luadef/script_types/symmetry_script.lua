---@meta 

---@class symmetry_mode
---@field saved_toml toml.table Modify this table to save data, at start it contains serialized data

---@class vehicle_editor.modify_interface
local modify_interface_type = {}
---@param allow_non_radial boolean If true, allows picking pieces which dont allow radial attachment
function modify_interface_type:start_picking_piece(allow_non_radial) end

---@type symmetry_mode The symmetry group being implemented
symmetry_mode = {}

---@return vehicle.piece
function symmetry_mode:get_root() end

---@type vehicle_editor.symmetry_panel
--- Not available if take_gui_control was not called!
symmetry_panel = nil

---@type vehicle_editor.modify_interface
--- Not available if take_gui_control was not called!
modify_interface = nil

---@type vehicle_editor.edveh_interface
--- Not available if take_gui_control was not called!
edveh_interface = nil

---@type vehicle
vehile = nil
