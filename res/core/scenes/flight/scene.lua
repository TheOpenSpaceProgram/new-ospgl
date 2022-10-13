-- The flight scene
local logger = require("logger")
require("renderer")
require("universe")
local glm = require("glm")
local cameras = dofile("core:scenes/cameras.lua")


-- We get optionally passed the entity id of vehicle to control
function load(veh_id)
    renderer:add_drawable(universe.system)

    for _, ent in pairs(universe.entities) do
        renderer:add_drawable(ent)
    end

    universe:sign_up_for_event("core:new_entity", 
        function (id) renderer:add_drawable(universe.get_entity(id)) end)
    
    universe:sign_up_for_event("core:remove_entity", 
        function (id) renderer:remove_drawable(universe.get_entity(id)) end)

end

function update(dt)
end

function render()
    renderer:render()
end

function unload()
    renderer.clear()
end

function get_camera_uniforms(width, height) 
    return cameras.from_pos_and_dir(0, 0)
end