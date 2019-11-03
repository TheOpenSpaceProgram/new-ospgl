
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

local sand = make_color(0.84,0.78,0.64)
local dirt = make_color(0.61,0.43,0.22)
local life_color = make_color(0.55,0.61,0.37);
local poles = make_color(1.0, 1.0, 1.0);

function life(x, y, z)

	noise.set_frequency(100.0);
	noise.set_fractal_octaves(8);


	return noise.perlin3_fractal(x, y, z) - 1.5;

end

function continents(x, y, z)

	noise.set_frequency(1.0);
	noise.set_fractal_octaves(14);
	noise.set_fractal_gain(0.6);

	return noise.perlin3_fractal(x, y, z) + 0.3;

end

function mountains(x, y, z)
	noise.set_frequency(100.0);
	noise.set_fractal_octaves(8);
	noise.set_fractal_gain(0.6);

	return math.abs(noise.perlin3_fractal(x, y, z));
end

function mountainf(x, y, z)

	noise.set_frequency(62.0);
	return noise.perlin3(x, y, z);
end

function polesfun(t)

	return clamp(1.0 - (1.0 / (1.5 * t * t * t * t)) + 0.5, 0.0, 1.0);

end

function desert(x, y, z)

	noise.set_frequency(5.0);
	noise.set_fractal_octaves(5);
	noise.set_fractal_rigidmulti();

	return noise.perlin3_fractal(x, y, z) - 0.05;

end


function generate()

	local pole_factor = polesfun(math.abs(coord_3d.y));
	local desert_factor = 1.0 - math.abs(coord_3d.y);

	local cf = continents(coord_3d.x, coord_3d.y, coord_3d.z);

	local lf = life(coord_3d.x, coord_3d.y, coord_3d.z) * (math.pow(math.abs(coord_3d.y), 2.0) + 0.5);
	lf = math.abs(lf);

	local df = desert(coord_3d.x, coord_3d.y, coord_3d.z) * desert_factor;

	--local mtf = clamp(mountainf(coord_3d.x, coord_3d.y, coord_3d.z), 0, 1) * clamp(cf - pole_factor, 0.0, 1.0);
	--local mt = mountains(coord_3d.x, coord_3d.y, coord_3d.z) * mtf;

	--local sur_color = mix_color(life_color, dirt, lf);
	--sur_color = mix_color(sur_color, sand, clamp(df, 0.0, 1.0));

	height = cf * radius * 0.01;
	--color = mix_color(sur_color, poles, pole_factor + mt * 2.0);
	color = life_color;
end

