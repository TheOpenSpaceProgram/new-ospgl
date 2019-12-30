local glm = require("glm")

-- Data set by caller:
--
--	coord_3d	-> vec3			-> Spherical 3D coordinates
--	coord_2d	-> vec2			-> Equirrectangular 2D coordinates (azimuth, elevation)
--	depth		-> int			-> Tile depth
--	radius		-> number		-> Radius of the planet

local hmap = get_heightmap("heightmap");
local cmap = get_image("colormap");


function generate()

	local earth = hmap.get_height_soft(coord_2d.x, coord_2d.y) - 0.0941;
	if earth < 0.0 then

		earth = earth - earth * earth * 160.0;
	end

	height = (earth * radius * 0.002);
	color = cmap.get_projected(coord_2d.x, coord_2d.y);

end

