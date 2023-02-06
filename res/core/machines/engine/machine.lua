-- Use this one as the machine
require("vehicle")
local assets = require("assets")


local icon = assets.get_image("machines/icons/engine.png")
function get_icon() return icon end

--require("machines/engine/flight.lua")

-- the global machine contains all OSP data and must contain all functions
-- engine is global as we are interested in everything having access to it
dofile("machines/engine/l_engine.lua")
plumbing = dofile("machines/engine/l_plumbing.lua")

