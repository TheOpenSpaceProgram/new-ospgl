
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

function ease_curve(x)
	return (3.0 * x) / (2.0 + math.abs(x * x));

end

function mountains(x, y, z)
	noise.set_fractal_fbm();
	noise.set_frequency(1.0);
	noise.set_fractal_octaves(8);
	noise.set_fractal_lacunarity(2.9232);
	local n1 = noise.simplex3_fractal(x, y, z);
	noise.set_fractal_rigidmulti();
	local n2 = noise.perlin3_fractal(x, y, z);

	return math.abs(ease_curve(n1 + n2) * n1 + math.abs(n2));
end

function continents(x, y, z)
	noise.set_fractal_rigidmulti();
	noise.set_frequency(1.2);
	noise.set_fractal_octaves(6);
	noise.set_fractal_lacunarity(2.3232);
	local n1 = noise.simplex3_fractal(x, y, z);

	local f = n1 * n1 * n1;

	return f;
end

function detail(x, y, z)

	local s = 15000.0;

	noise.set_fractal_fbm();
	noise.set_frequency(1.0);
	noise.set_fractal_octaves(4);
	noise.set_fractal_lacunarity(2.0232);
	local n1 = noise.simplex3_fractal(x * s, y * s, z * s);

	local f = n1;

	return f;
end


function generate()
	
	local c = continents(coord_3d.x, coord_3d.y, coord_3d.z);
	local m = mountains(
	coord_3d.x + c * 0.005, 
	coord_3d.y + c * 0.005, 
	coord_3d.z + c * 0.005);
	
	local m_pow = m * c * c;

	local d = detail(coord_3d.x, coord_3d.y, coord_3d.z);

	return ((m * (1.0 - m_pow)) * 100000.0 * (1.0 - math.abs(c)) - 
	math.abs(c) * 5000.0 - math.abs(d) * 50.0 - 8000.0) * 0.55;
end
