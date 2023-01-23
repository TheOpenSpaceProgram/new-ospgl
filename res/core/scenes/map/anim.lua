--- Animates the loading / unloading of the map to make the transition seamless
--- Allows easy disabling through the config subsystem for quicker switching

---@class core.map.anim
local intro_anim = {}

--- Animates the camera going away from the vehicle in the direction of the camera
function intro_anim.from_flight(cam_pos, cam_dir) end

--- Animates the camera going away from the planet through the normal
function intro_anim.from_planet(cam_pos, cam_dir, normal) end

return intro_anim
