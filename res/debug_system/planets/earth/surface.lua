local glm = require("glm")
local assets = require("assets")
local noise = require("noise")

local ngen = noise.new(987)

local grass = glm.vec3.new(0.196, 0.275, 0.027)
local grass1 = glm.vec3.new(0.347, 0.420, 0.255)
local desert = glm.vec3.new(0.480, 0.347, 0.255)
local snow = glm.vec3.new(0.5, 0.5, 0.5)
local rock = glm.vec3.new(0.298, 0.373, 0.288)

function continent_generator(coord)

	ngen:set_fractal_octaves(8)
	ngen:set_fractal_type(noise.fbm)
	local noise0 = ngen:get_perlin_fractal((coord * 60.0):unpack()) * 3.0
	ngen:set_fractal_octaves(3)
	local ocean_offset = glm.vec3.new(0.4, 0.4, 0.4)
	local oceans = ngen:get_perlin_fractal(((coord + ocean_offset) * 40.0):unpack())

	local v = noise0 - math.max(oceans, 0.0)
	local surf = math.max(v, 0.0)

	return v * 0.9 + surf * surf * 2.0
end 

function cont_shape_generator(coord, continents)

	ngen:set_fractal_type(noise.rigid_multi)
	ngen:set_fractal_octaves(6)
	local noise0 = ngen:get_perlin_fractal((coord * 3000.0):unpack()) + 0.9

	return math.max(noise0, 0.0) * math.max(continents, 0.0)
end 

function desert_factor_generator(coord)

	local yabs = glm.abs(coord.y)
	local yval = 1.0 - yabs * yabs 
	yval = glm.min(glm.pow(yval, 10.0) * 2.0, 1.0)

	local off = glm.vec3.new(1.0, 1.0, 1.0)
	ngen:set_fractal_octaves(3)
	local noise = math.abs(ngen:get_perlin_fractal(((coord + off) * 40.0):unpack()))

	return math.min(yval * noise * noise * 10.0, 1.0)

end 

function mountain_generator(coord, continents, desert_factor)

	ngen:set_fractal_octaves(3)

	local off = glm.vec3.new(4.0, 0.0, 4.0)
	local n = math.abs(ngen:get_perlin_fractal(((coord + off) * 1000.0):unpack()))
	n = math.max(math.min(n * n * n * 10.0, 1.0) - desert_factor * 0.6, 0.0)

	local mval = math.max(n * continents, 0.0)

	ngen:set_fractal_octaves(6)
	ngen:set_fractal_type(noise.rigid_multi)
	local off2 = glm.vec3.new(0.0, 1.0, 0.0)
	local mountains = ngen:get_perlin_fractal(((coord + off2) * 30000.0):unpack())

	return math.min((mountains + 0.5) * mval * 3.0, 1.0)

end 

function dune_generator(coord, continents, desert_factor)

	ngen:set_fractal_octaves(5)
	ngen:set_fractal_type(noise.billow)
	local off = glm.vec3.new(0.0, 0.0, 0.0)
	local dunes = ngen:get_perlin_fractal(((coord + off) * 60000.0):unpack())
	dunes = dunes * dunes * 2.0

	return math.min(dunes * desert_factor * continents * 7.0, 1.0)
	
end

function depth_generator(coord, continents)

	ngen:set_fractal_octaves(4)
	ngen:set_fractal_type(noise.rigid_multi)
	local off = glm.vec3.new(-1.0, -1.0, -1.0)
	local depths = ngen:get_perlin_fractal(((coord + off) * 200.0):unpack())
	local off2 = glm.vec3.new(0.0, -1.0, 0.0)
	local depths2 = ngen:get_perlin_fractal(((coord + off2) * 300.0):unpack())

	return math.max(depths * 6.0 * -glm.min(continents, -0.5) + glm.abs(depths2) * -glm.min(continents, 0.0) * 4.0, 0.0)
end 

function detail_generator(coord, desert_factor, mountains)

	ngen:set_fractal_octaves(4)
	ngen:set_fractal_type(noise.fbm)

	local off = glm.vec3.new(4.0, 0.0, -4.0)
	local details = ngen:get_perlin_fractal(((coord + off) * 1000.0):unpack()) * 0.6

	local extra = ngen:get_perlin_fractal(((coord + off) * 2000.0):unpack()) * 1.7

	return details + extra * math.max(mountains - desert_factor, 0.0)
end

function generate(info, out)

	local continents = continent_generator(info.coord_3d)
	local cont_shape = cont_shape_generator(info.coord_3d, continents)
	local desert_factor = desert_factor_generator(info.coord_3d)
	
	local mountains = mountain_generator(info.coord_3d, continents, desert_factor)
	local dunes = dune_generator(info.coord_3d, continents, desert_factor)

	local depths = depth_generator(info.coord_3d, continents)

	local details = detail_generator(info.coord_3d, desert_factor, mountains)

	local h = cont_shape * 7000.0 - 1000.0 - depths * 8000.0 + mountains * 8000.0 + dunes * 2500.0 + details * 150.0

	--out.height = h + 1000.0
	out.height = h

	local rock_factor = math.min(math.max(mountains, 0.0) * 2.0, 1.0)
	local grass_factor = 1.0 - rock_factor

	local grass_color = grass * (1.0 - desert_factor) + desert * desert_factor;
	local snow_factor = math.max(math.min(12.0 * ((h / 8000.0) - 0.9), 1.0), 0.0);
	local rock_color = rock * (1.0 - snow_factor) + snow * snow_factor;
	
	out.color = rock_factor * rock_color + grass_color * grass_factor

	--out.color = glm.vec3.new(h / 10000.0, h / 10000.0, h / 10000.0)
end

