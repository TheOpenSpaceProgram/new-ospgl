---@meta
local assets = {}

---@class bitmap_font_handle
local bitmap_font_handle = {}
--- Invalidates current handle, and returns a new one
---@return bitmap_font_handle 
function bitmap_font_handle:move() end
---@return bitmap_font
function bitmap_font_handle:get() end

---@class cubemap_handle
local cubemap_handle = {}
--- Invalidates current handle, and returns a new one
---@return cubemap_handle 
function cubemap_handle:move() end
---@return cubemap
function cubemap_handle:get() end

---@class image_handle
local image_handle = {}
--- Invalidates current handle, and returns a new one
---@return image_handle 
function image_handle:move() end
---@return image
function image_handle:get() end

---@class material_handle
local material_handle = {}
--- Invalidates current handle, and returns a new one
---@return material_handle 
function material_handle:move() end
---@return material
function material_handle:get() end

---@class model_handle
local model_handle = {}
--- Invalidates current handle, and returns a new one
---@return model_handle 
function model_handle:move() end
---@return model
function model_handle:get() end

---@class physical_material_handle
local physical_material_handle = {}
--- Invalidates current handle, and returns a new one
---@return physical_material_handle 
function physical_material_handle:move() end
---@return physical_material
function physical_material_handle:get() end

---@param path string
---@return bitmap_font_handle
function assets.get_bitmap_font(path) end

---@param path string
---@return cubemap_handle
function assets.get_cubemap(path) end

---@param path string
---@return image_handle
function assets.get_image(path) end

---@param path string
---@return material_handle
function assets.get_material(path) end

---@param path string
---@return model_handle
function assets.get_model(path) end

---@param path string
---@return physical_material_handle
function assets.get_physical_material(path) end


return assets
