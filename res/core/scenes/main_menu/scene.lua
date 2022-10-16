-- This scene is unique in the fact that it's always loaded if scene is null
-- the first time receiving any "load_state = [x]" and skipping directly to the 
-- scene determined in the save
-- Note that packages have not been initialized (except for core)!
local scene = require("scene")

function load()

end

function bootstrap()
    scene.load("scenes/flight/scene.lua")
end