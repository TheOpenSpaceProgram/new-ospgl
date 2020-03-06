local glm = require("glm")
local assets = require("assets")
local noise = require("noise")

local hmap = assets.get_image("rss_textures:earth/hmap.png")
local cmap = assets.get_image("rss_textures:earth/color.png")
local ngen = noise.new(1234)

function projected_to_pixel(prj)

	return glm.vec2.new(prj.x / glm.two_pi + 0.5, prj.y / glm.pi);

end

function generate(info, out)

	local pix = projected_to_pixel(info.coord_2d);
	local earth = hmap:get():sample_bilinear(pix).x - 0.0941;
	if earth < 0.0 then

		earth = earth - earth * earth * 160.0;
	end

	local freq = 500000.0;
	local nfreq = 100000.0;

	local noisyness = ngen:get_perlin((info.coord_3d * nfreq):unpack())
	local noise0 = ngen:get_perlin_fractal((info.coord_3d * freq):unpack())


	out.height = earth * 8000.0 + noise0 * 200.0 * glm.abs(noisyness);
	out.color = cmap:get():sample_bilinear(pix):to_vec3();
end

