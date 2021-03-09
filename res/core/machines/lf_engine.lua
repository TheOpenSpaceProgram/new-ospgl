-- A Liquid Fuel Engine accepts a given number of fluid ports
-- all of which lead to the reaction chamber where they are
-- chemically reacted and the exhaust driven through a nozzle
-- Each input may either be pressure-fed, or pump-fed. We don't go into
-- details on how the pumping is done, that could be implemented later on.


require("toml")
local assets = require("assets")

local icon = assets.get_image("core:machines/icons/engine.png")
function get_icon() return icon end
