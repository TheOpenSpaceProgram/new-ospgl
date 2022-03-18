-- Use this one as the machine
local vehicle = require("vehicle")
local assets = require("assets")
local logger = require("logger")

local icon = assets.get_image("machines/icons/fluid_tank.png")
function get_icon() return icon end

plumbing = dofile("machines/fuel_tank/plumbing.lua")

function update(dt)
    plumbing.update(dt)

end