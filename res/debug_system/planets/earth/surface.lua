local glm = require("glm")
local assets = require("assets")
local noise = require("noise")

local ngen = noise.new(1234);
ngen:set_fractal_octaves(12);

local grass = glm.vec3.new(0.196, 0.275, 0.027);
local grass1 = glm.vec3.new(0.447, 0.420, 0.255);

function generate(info, out)


	local noise0 = ngen:get_perlin_fractal((info.coord_3d * 250.0):unpack());
	local noise1 = ngen:get_perlin((info.coord_3d * 10000.0):unpack());
	local noise2 = ngen:get_perlin((info.coord_3d * 100000.0):unpack());

	out.height = (noise0 * 17000.0 + noise1 * 4000.0 + noise2 * 500.0) - 300.0;
	--out.height = 1200.0
	--if info.coord_3d.x >= 0.0 then
	--	out.height = 900.0
	--else
	--
	--end

	local gval = (glm.abs(noise1) + glm.abs(noise2)) * 1.0;

	out.color = grass * gval + grass1 * (1.0 - gval);
end

