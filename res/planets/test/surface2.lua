
-- Returns named items modificable by the user here 
-- Types can be:
-- num, vec2, vec3, color
function get_data_items()
	return {}
end

-- Data set by caller:
--
--	coord_3d	-> {x, y, z}	-> Spherical 3D coordinates 
--	coord_2d	-> {x, y}		-> Equirrectangular 2D coordinates (azimuth, elevation)
--	depth		-> int			-> Tile depth
--	radius		-> float		-> Radius of the planet
--	data		-> table		-> Data set by the user
--
-- Also, all utility terrain generating functions can be used
-- You should return height at said point in meters, can be negative

function biome(x, y, z)

	noise.set_cellular_distance();
	local d = noise.cellular3(x, y, z);
	noise.set_cellular_distance();
	local n = noise.perlin3(x, y, z);

	local d2 = noise.cellular3(x + n * 0.02, y + n * 0.3, z);

	return d * d2 ^ 0.2;

end

function mountains(x, y, z)
	noise.set_frequency(5.1);
	noise.set_fractal_octaves(9);
	local n = noise.simplex3_fractal(x, y, z);
	return n;
end

function sea(x, y, z)
	return -20.0;
end

function plains(x, y, z)
	noise.set_frequency(1.1);
	noise.set_fractal_octaves(6);
	local n = noise.perlin3_fractal(x, y, z);
end

function combine_biomes(x, y, z, b)
	
	local m = b * mountains(x, y, z);
	local p = (1.0 - b) * mountains(x, y, z);
	local s = b * b * sea(x, y, z);

	return m + p + s;

end

function generate()
	local c = coord_3d;
	local biomes = biome(c.x, c.y, c.z);

	local cmb = combine_biomes(c.x, c.y, c.z, biomes);

	return cmb * 10000.0 + 10000.0;
end
