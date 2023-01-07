---@meta 


local flight_input = {}

---@class flight_input.context
flight_input.context = {}

---@return flight_input.context
---@nodiscard
---@param res string Path to the flight input resource
function flight_input.context.new(res) end

---@return number
---@param name string 
---@nodiscard
--- Returns value of axis
function flight_input.context:get_axis(name) end
---@return boolean
---@param name string 
---@nodiscard
--- Returns true if action is currently active
function flight_input.context:get_action(name) end
---@return boolean
---@param name string 
---@nodiscard
--- Returns true if action was deactivated this frame
function flight_input.context:get_action_down(name) end
---@return boolean
---@param name string 
---@nodiscard
--- Returns true if action was activated this frame
function flight_input.context:get_action_up(name) end
---@param name string 
---@param val number
---@param epsilon number
--- Sets given axis to a value.
--- If the axis is joystick controlled, we obviously can't control
--- the value of said axis. The solution is ignoring the physical value
--- from the joystick until it comes within epsilon of the set value.
--- If you use val = 1.0 or val = 0.0, you may leave epsilon at 0 as these 
--- can always be reached.
function flight_input.context:set_axis(name, val, epsilon) end

return flight_input
