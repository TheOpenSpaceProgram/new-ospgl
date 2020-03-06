local logger = require("logger")
local glm = require("glm")
local noise_lib = require("noise")
local assets = require("assets")

local img = assets.get_image("rss_textures:moon/color.png")
print(img:get():sample_bilinear(0.05, 0.05))