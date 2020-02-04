local glm = require("glm")
local assets = require("assets")
local noise = require("noise")

local ngen = noise.new(1234);

function generate(info, out)



	local noisyness = ngen:get_perlin((info.coord_3d * 10.0):unpack())
	local noise0 = ngen:get_perlin_fractal((info.coord_3d * 1000.0):unpack())


	out.height = noise0 * 200.0 * glm.abs(noisyness);
	out.color = glm.vec3.new(0.3, 0.3, 0.3);
end

