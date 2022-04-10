-- Use this one as the machine
local vehicle = require("vehicle")
local assets = require("assets")
local logger = require("logger")
local imgui = require("imgui")

local icon = assets.get_image("machines/icons/fluid_tank.png")
function get_icon() return icon end

plumbing = dofile("machines/fuel_tank/plumbing.lua")

local debug_pause = true
local pause_reaction = true

local function rnts(n)
    return string.format("%.2f", n)
end

function draw_imgui()
    local contents = plumbing.fluid_container.contents:get_contents();
    local tmass = 0.0
    imgui.text("Settings:")
    debug_pause = imgui.checkbox("Pause", debug_pause)
    pause_reaction = imgui.checkbox("Pause Reactions", pause_reaction)
    imgui.text("Contents:")
    for phys_mat, stored_fluid in contents:pairs() do
        imgui.text(phys_mat.name)
        imgui.bullet_text("Liquid: " .. rnts(stored_fluid.liquid_mass) .. "kg")
        imgui.bullet_text("Gas: " .. rnts(stored_fluid.gas_mass) .. "kg")
        imgui.bullet_text("Total: " .. rnts(stored_fluid.liquid_mass + stored_fluid.gas_mass) .. "kg")
        tmass = tmass + stored_fluid.liquid_mass + stored_fluid.gas_mass
        imgui.separator()
    end
    imgui.text("Properties: ")
    imgui.bullet_text("T: " .. rnts(plumbing.fluid_container.temperature) .. "K")
    imgui.bullet_text("P: " .. rnts(plumbing.fluid_container:get_pressure() / 101300) .. "atm")
    imgui.bullet_text("Contents mass: " .. rnts(tmass) .. "kg")
    imgui.bullet_text("V: " .. rnts(plumbing.fluid_container.volume) * 1000.0 .. "L")
end

function update(dt)

    if not debug_pause then
        plumbing.update(dt, not pause_reaction)
    end

end