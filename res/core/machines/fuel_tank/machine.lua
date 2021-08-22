-- Use this one as the machine
local vehicle = require("vehicle")
local assets = require("assets")

print("Guamedo: ")
print(vehicle.vehicle)
print(vehicle.piece)
print(vehicle.part)
print(vehicle.machine)
print(vehicle.stored_fluids)

local icon = assets.get_image("machines/icons/engine.png")
function get_icon() return icon end

plumbing = dofile("machines/fuel_tank/plumbing.lua")
