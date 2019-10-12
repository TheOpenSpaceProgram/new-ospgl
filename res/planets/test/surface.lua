
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

function generate()
	local factor = perlin3d(
	coord_3d.x * 10.0,
	coord_3d.y * 10.0,
	coord_3d.z * 10.0,
	16);

	local a = 100.0;
	local b = 50.0;

	local scale = a * factor + b * (1.0 - factor);
	local scale_y = a * (1.0 - factor) + b * factor;
	local scale_z = scale - scale_y;
	local dunes = perlin3d(
	coord_3d.x * scale,
	coord_3d.y * scale_y,
	coord_3d.z * scale_z,
	14);

	return dunes * 1000.0;
end
















































































































































































































