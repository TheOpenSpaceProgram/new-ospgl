-- Use this one as the machine
local assets = require("assets")

local icon = assets.get_image("machines/icons/engine.png")
function get_icon() return icon end

local nvg = require("nano_vg")
