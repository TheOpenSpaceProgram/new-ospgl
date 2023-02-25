---@module "editor_script"
-- This is technically not a scene, but simply a script called from the editor
-- to allow customizing certain behaviour
-- A global, "editor" of type editor_scene is passed

require("editor")
local events = dofile("util/c_events.lua")

events:add(editor.veh, "on_lost_piece", on_lost_piece)
events:add(editor.int, "on_right_click", on_right_click)

function on_right_click(piece_id)

end

function on_lost_piece(id)

end
