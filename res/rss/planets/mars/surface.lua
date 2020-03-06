local glm = require("glm")
local assets = require("assets")

local hmap = assets.get_image("rss_textures:mars/hmap.png")
local cmap = assets.get_image("rss_textures:mars/color.png")

function projected_to_pixel(prj)

	return glm.vec2.new(prj.x / glm.two_pi + 0.5, prj.y / glm.pi);

end

function generate(info, out)

	local pix = projected_to_pixel(info.coord_2d);
	local earth = hmap:get():sample_bilinear(pix).x;


	out.height = (earth * 200.0);
	out.color = cmap:get():sample_bilinear(pix):to_vec3();

end

