---@meta 

---@class vehicle.plumbing_machine
local plumbing_machine = {} 
---@param id string 
---@param marker string 
---@param ui_name string Displayed to the user, use localization!
---@param is_flow_port boolean 
---@param x number
---@param y number
function plumbing_machine:create_port(id, marker, ui_name, is_flow_port, x, y) end

--
---@class plumbing.fluid_map
local fluid_map = {}
---@return assets.physical_material mat
---@return plumbing.stored_fluid
function fluid_map:pairs() end

---@class plumbing.stored_fluid
---@field liquid_mass number
---@field gas_mass number
local stored_fluid = {}

---@return number
function stored_fluid:get_total_mass() end

---@class plumbing.stored_fluids
---@field temperature number
local stored_fluids = {}


---@return 
function stored_fluids:get_contents() end

---@param mat assets.physical_material_handle
---@param liquid number liquid mass to add
---@param gas number gas mass to add
---@param temp number Leave negative to add at same temperature as tank
function stored_fluids:add_fluid(mat, liquid, gas, temp) end

---@param mat assets.physical_material
---@param liquid number liquid mass to set
---@param gas number gas mass to set
function stored_fluids:modify_fluid(mat, liquid, gas) end

---@param mat assets.physical_material
---@param factor number
function stored_fluids:set_vapor_fraction(mat, factor) end

---@param b plumbing.stored_fluids
---@return plumbing.stored_fluids
--- Modifies with a second stored fluids, which may have negative
--- values to take away and return
function stored_fluids:modify(b) end

---@param by number
---@return plumbing.stored_fluids
function stored_fluids:multiply(by) end

---@param target plumbing.stored_fluids
---@param mat assets.physical_material
---@param liquid_mass number
---@param gas_mass number
---@param do_flow boolean 
--- No negatives needed as this assumes draining
function stored_fluids:drain_to(target, mat, liquid_mass, gas_mass, do_flow) end

---@return number
function stored_fluids:get_total_liquid_volume() end

---@return number
function stored_fluids:get_total_liquid_mass() end

---@return number
function stored_fluids:get_total_gas_mass() end

---@param heat number
function stored_fluids:add_heat(heat) end

---@return number
function stored_fluids:get_total_gas_moles() end

---@return number
function stored_fluids:get_average_liquid_density() end

---@return number
function stored_fluids:get_specific_gas_constant() end

---@return number Heat released
---@param T number Temperature of the reaction
---@param react_V number Volume used for the chemical reaction
---@param liquid_react_factor number How fast do liquids react relative to gases
---@param dt number Delta time step
function stored_fluids:react(T, react_V, liquid_react_factor, dt) end
