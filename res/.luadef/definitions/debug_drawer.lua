---@meta 


local debug_drawer = {}

---@param p glm.vec3 location of the point
---@param color glm.vec3 color of the point in RGB 
function debug_drawer.add_point(p, color) end

---@param p0 glm.vec3 Start point of the line
---@param p1 glm.vec3 End point of the line
---@param color glm.vec3 Start point of the line
---@param bcolor? glm.vec3 End point of the line
--- If only one color is given, the line is only a single color
function debug_drawer.add_line(p0, p1, color, bcolor) end

---@param p0 glm.vec3 Start point of the arrow 
---@param p1 glm.vec3 End point of the arrow 
---@param color glm.vec3 Color of the arrow 
function debug_drawer.add_arrow(p0, p1, color) end

---@param base glm.vec3 Start point of the cone 
---@param tip glm.vec3 End point of the cone 
---@param radius number  Radius at the base
---@param color glm.vec3 Color of the cone
function debug_drawer.add_cone(base, tip, radius, color) end

return debug_drawer
