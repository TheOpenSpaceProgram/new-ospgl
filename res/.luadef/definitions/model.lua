---@meta 

---@class model.node
---@field children model.node[]
---@field name string
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

---@return model.node[]
function node:get_children_recursive() end

---@param name string
---@return model.node
--- Allows obtaining "children of children"
function node:get_child_deep(name) end

--- @param child model.node Children to get bounds of in our coordinate system
function node:get_bounds_of(child) end
---@param child model.node
---@return glm.mat4
--- Obtains the transform matrix that goes from our space to children-node space, use it to
--- position colliders, etc...
--- Try to cache the result, as this operation could be expensive on complex models
function node:get_tform_of(child) end

---@return bullet.collision_shape Collider
---@return glm.mat4 Collider transform
---@param n model.node Node to extract collider from, assumed to have a collider!
--- Forms a collision shape from the node using model properties (see documentation of modelling)
--- Make sure target node is meant to have a collider, as otherwise this will throw an error
--- Note: The collider is generated in n coordinates, so to properly place the rigidbody it's on, you may 
--- either create a compound collider (and offset it inside) or simply place the rigidbody offset by the returned matrix
--- The returned matrix transforms from the collider space to the space of the node being called
function node:extract_collider(n) end


---@class model.model
local model = {}

---@param name? string If not given, returns root node
---@return model.node|nil
--- The node may not be ready to draw as it may not be on the GPU! To draw nodes
--- use gpu_pointer. This is meant to be used to extract colliders, etc...
function model:get_node(name) end

---@return model.gpu_pointer
function model:get_gpu() end

---@class model.gpu_pointer
local gpu_pointer = {}

--- Invalidates current pointer, and returns a new one
---@return model.gpu_pointer 
function gpu_pointer:move() end

---@param name? string Name of the node. If not given, returns root node
---@return model.node|nil
--- It's guaranteed this node is ready to draw and is uploaded to the GPU. Don't store the raw pointer for long!
function gpu_pointer:get_node(name) end
