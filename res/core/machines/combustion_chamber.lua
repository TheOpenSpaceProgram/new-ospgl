-- A combustion chamber accepts any number of fluid ports
-- and allows chemical reactions between them. It can have an ignitor
-- system to allow the starting of high activation energy reactions
-- (non-hypergolic) such as a spark-gap or pyrotechnic charge
require("toml")
local assets = require("assets")

local icon = assets.get_image("core:machines/icons/engine.png")
function get_icon() return icon end
