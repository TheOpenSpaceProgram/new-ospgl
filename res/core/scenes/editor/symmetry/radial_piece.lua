---@module "symmetry_script"
-- Radial around a piece symmetry
local wiring = dofile("core:scenes/editor/symmetry/c_wiring.lua"):init(saved_toml)
local plumbing = dofile("core:scenes/editor/symmetry/c_plumbing.lua"):init(saved_toml)

---@param p vehicle.piece
function on_dirty(p)
	wiring:on_dirty(p) 
	plumbing:on_dirty(p)
end

---@param layout gui.vertical_layout
function editor_widgets(layout)
	wiring:editor_widgets(layout)
	wiring:editor_widgets(layout)
end

