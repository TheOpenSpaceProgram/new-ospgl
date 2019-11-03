
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

local poles = make_color(0.9, 0.9, 0.9);
local h_color = make_color(0.9921, 0.5215, 0.3764);
local l_color = make_color(0.4196, 0.3098, 0.2627);

function polesfun(t)

	return clamp(1.0 - (1.0 / (1.5 * t * t * t * t)) + 0.5, 0.0, 1.0);

end

function hval(x, y, z)

	noise.set_frequency(100.0);
	noise.set_fractal_octaves(10);
	noise.set_fractal_gain(0.6);

	local n = noise.perlin3_fractal(x, y, z);

	return clamp(math.pow(n, 0.7), 0.0, 1.0);

end

function surface(x, y, z)

	noise.set_frequency(3.0);
	noise.set_fractal_octaves(10);
	noise.set_fractal_gain(0.6);

	local n = noise.perlin3_fractal(x, y, z);

	return clamp(n * 2.0, -1.0, 1.0);

end

function generate()
	local pole_factor = polesfun(math.abs(coord_3d.y));
	local s = surface(coord_3d.x, coord_3d.y, coord_3d.z) +
		1.0 - clamp(math.abs(coord_3d.y) * 2.5, 0.0, 1.0);
	local s2 = hval(coord_3d.x, coord_3d.y, coord_3d.z);

	local s_color = mix_color(h_color, l_color, clamp(s, 0.0, 1.0));

	height = s2 * radius * 0.001;

	color = mix_color(s_color, poles, pole_factor + math.max(s2, 0.0) * 0.1);
end

