---@meta
local assets = {}

---@class assets.bitmap_font_handle
local bitmap_font_handle = {}
--- Invalidates current handle, and returns a new one
---@return assets.bitmap_font_handle 
function bitmap_font_handle:move() end
---@return assetts.bitmap_font
function bitmap_font_handle:get() end

---@class assets.cubemap_handle
local cubemap_handle = {}
--- Invalidates current handle, and returns a new one
---@return assets.cubemap_handle 
function cubemap_handle:move() end
---@return assets.cubemap
function cubemap_handle:get() end

---@class assets.image_handle
local image_handle = {}
--- Invalidates current handle, and returns a new one
---@return assets.image_handle 
function image_handle:move() end
---@return assets.image
function image_handle:get() end

---@class assets.material_handle
local material_handle = {}
--- Invalidates current handle, and returns a new one
---@return assets.material_handle 
function material_handle:move() end
---@return assets.material
function material_handle:get() end

---@class assets.model_handle
local model_handle = {}
--- Invalidates current handle, and returns a new one
---@return assets.model_handle 
function model_handle:move() end
---@return assets.model
function model_handle:get() end

---@class assets.physical_material_handle
local physical_material_handle = {}
--- Invalidates current handle, and returns a new one
---@return assets.physical_material_handle 
function physical_material_handle:move() end
---@return assets.physical_material
function physical_material_handle:get() end

---@param path string
---@return assets.bitmap_font_handle
function assets.get_bitmap_font(path) end

---@param path string
---@return assets.cubemap_handle
function assets.get_cubemap(path) end

---@param path string
---@return assets.image_handle
function assets.get_image(path) end

---@param path string
---@return assets.material_handle
function assets.get_material(path) end

---@param path string
---@return assets.model_handle
function assets.get_model(path) end

---@param path string
---@return assets.physical_material_handle
function assets.get_physical_material(path) end


return assets
