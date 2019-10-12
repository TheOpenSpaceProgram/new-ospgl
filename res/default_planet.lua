
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
	return 0.0;
end

