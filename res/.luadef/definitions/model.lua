---@meta 

---@class model.node
local node = {}

---@param c_uniforms renderer.camera_uniforms Camera uniforms from the camera
---@param model glm.mat4 Model matrix
---@param did integer Integer used for per-model effects, leave as 0 if you don't know what to set
---@param ignore_our_subtform boolean Should our subtform be ignored? Use if you are drawing a children node directly, otherwise false
---@param increase_did boolean Should did be increased when drawing children? Use to share per-model effects across children
function node:draw(c_uniforms, model, did, ignore_our_subtform, increase_did) end

---@param sh_cam renderer.shadow_camera Camera uniforms for the shadow camera 
---@param model glm.mat4 Model matrix 
---@param ignore_our_subtform boolean Should our subtform be ignored? Use if you are drawing a children node directly, otherwise false
function node:draw_shadow(sh_cam, model, ignore_our_subtform) end
