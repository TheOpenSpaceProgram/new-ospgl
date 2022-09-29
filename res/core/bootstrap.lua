-- This file is unique to the "core" package and called right after loading
-- its purpose is loading a scene from which the game afterwards develops
local scene = require("scene")

function bootstrap()
    scene.load("scenes/flight/scene.lua")
end