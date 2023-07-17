---@meta 

---@class vehicle_editor.modify_interface
local modify_interface_type = {}
---@param allow_non_radial boolean If true, allows picking pieces which dont allow radial attachment
---@param forbidden_pieces vehicle.piece[] A list of pieces which may not be picked (a good default in symmetry_mode:get_all_pieces())
function modify_interface_type:start_picking_piece(allow_non_radial, forbidden_pieces) end


---@type symmetry_mode The symmetry group being implemented
symmetry_mode = {}

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
vehicle = nil

---@type vehicle_editor.vehicle
editor_vehicle = nil
