#pragma once
#include <sol.hpp>
#include <glm/glm.hpp>
#include "../../universe/body/config/PlanetConfig.h"

class LuaUtilLib
{
public:

	static void load_lib(sol::state& lua_state, PlanetConfig* config)
	{
		lua_state["make_color"] = [&lua_state](double r, double g, double b)
		{
			sol::table out_table = lua_state.create_table_with("r", r, "g", g, "b", b);
			return out_table;
		};

		lua_state["mix_color"] = [&lua_state](sol::table a, sol::table b, double t)
		{
			sol::table out_table = lua_state.create_table_with(
				"r", a["r"].get_or(0.0) * (1.0 - t) + b["r"].get_or(0.0) * t, 
				"g", a["g"].get_or(0.0) * (1.0 - t) + b["g"].get_or(0.0) * t, 
				"b", a["b"].get_or(0.0) * (1.0 - t) + b["b"].get_or(0.0) * t);

			return out_table;

		};

		lua_state["clamp"] = [](double val, double min, double max)
		{
			return glm::clamp(val, min, max);
		};

	
		// Images are always casted to 4 channel 8 bit color
		lua_state["get_image"] = [&lua_state, config](const std::string& name)
		{
			sol::table out_image = lua_state.create_table();

			Image* img = nullptr;
			auto it = config->surface.images.find(name);
			if (it == config->surface.images.end())
			{
				logger->error("Tried to get a image which did not exist");

			}
			else
			{
				if (it->second->get_config().channels != 3 || it->second->get_config().bitdepth != 8)
				{
					logger->error("Tried to get a image which is a heightmap, make sure it has channel = 3 and bitdepth = 8");
				}
			}

			img = it->second;

			out_image["get_projected"] = [&lua_state, img](float x, float y)
			{
				x += glm::pi<float>();
				x /= glm::two_pi<float>();
				y /= glm::pi<float>();

				int w = img->get_width();
				int h = img->get_height();

				glm::vec2 cf = glm::vec2(x * w, y * h);

				float x1 = floor(x * (float)w);
				float x2 = ceil(x * (float)w);
				float y1 = floor(y * (float)h);
				float y2 = ceil(y * (float)h);



				glm::vec2 tl = glm::vec2(x1, y1);
				glm::vec2 tr = glm::vec2(x2, y1);
				glm::vec2 bl = glm::vec2(x1, y2);
				glm::vec2 br = glm::vec2(x2, y2);

				int tli = img->get_index(MathUtil::int_repeat((int)tl.x, w - 1), MathUtil::int_clamp((int)tl.y, h - 1));
				int tri = img->get_index(MathUtil::int_repeat((int)tr.x, w - 1), MathUtil::int_clamp((int)tr.y, h - 1));
				int bli = img->get_index(MathUtil::int_repeat((int)bl.x, w - 1), MathUtil::int_clamp((int)bl.y, h - 1));
				int bri = img->get_index(MathUtil::int_repeat((int)br.x, w - 1), MathUtil::int_clamp((int)br.y, h - 1));
				auto[tlsr, tlsg, tlsb] = img->get_rgb_unsafe<uint8_t>(tli);
				auto[trsr, trsg, trsb] = img->get_rgb_unsafe<uint8_t>(tri);
				auto[blsr, blsg, blsb] = img->get_rgb_unsafe<uint8_t>(bli);
				auto[brsr, brsg, brsb] = img->get_rgb_unsafe<uint8_t>(bri);

				glm::vec3 tls = glm::vec3(tlsr / 255.0f, tlsg / 255.0f, tlsb / 255.0f);
				glm::vec3 trs = glm::vec3(trsr / 255.0f, trsg / 255.0f, trsb / 255.0f);
				glm::vec3 bls = glm::vec3(blsr / 255.0f, blsg / 255.0f, blsb / 255.0f);
				glm::vec3 brs = glm::vec3(brsr / 255.0f, brsg / 255.0f, brsb / 255.0f);

				// Interpolate in X, which we use as the base interp
				float x_point = (cf.x - x1) / (x2 - x1);

				glm::vec3 xtop = glm::mix(tls, trs, x_point);
				glm::vec3 xbot = glm::mix(bls, brs, x_point);

				// Interpolate in Y, between xtop and xbot
				float y_point = (cf.y - y1) / (y2 - y1);
				glm::vec3 interp;

				if (x1 == x2 && y1 == y2)
				{
					// Return any of them
					interp = tls;
				}
				else if (x1 == x2) 
				{
					// Interpolate in y
					interp = glm::mix(tls, bls, y_point);
				}
				else if (y1 == y2)
				{
					// Interpolate in x
					interp = glm::mix(tls, trs, x_point);
				}
				else
				{
					interp = glm::mix(xtop, xbot, y_point);
				}


				return lua_state.create_table_with("r", interp.x, "g", interp.y, "b", interp.z);

			};

			return out_image;

		};

		// Heightmaps are always 1 channel 16 bit color
		lua_state["get_heightmap"] = [&lua_state, config](const std::string& name)
		{
			sol::table out_image = lua_state.create_table();

			Image* img = nullptr;
			auto it = config->surface.images.find(name);
			if (it == config->surface.images.end())
			{
				logger->error("Tried to get a heightmap which did not exist");
				
			}
			else
			{
				if (it->second->get_config().channels != 1 || it->second->get_config().bitdepth != 16)
				{
					logger->error("Tried to get a heightmap which is a image, make sure it has channel = 1 and bitdepth = 16");
				}
			}

			img = it->second;
			
			// Interpolated pixel getter, assumes tiling x and clamped y
			out_image["get_height_soft"] = [&lua_state, img](float x, float y)
			{

				x += glm::pi<float>();
				x /= glm::two_pi<float>();
				y /= glm::pi<float>();

				int w = img->get_width();
				int h = img->get_height();

				glm::vec2 cf = glm::vec2(x * w, y * h);

				float x1 = floor(x * (float)w);
				float x2 = ceil(x * (float)w);
				float y1 = floor(y * (float)h);
				float y2 = ceil(y * (float)h);



				glm::vec2 tl = glm::vec2(x1, y1);
				glm::vec2 tr = glm::vec2(x2, y1);
				glm::vec2 bl = glm::vec2(x1, y2);
				glm::vec2 br = glm::vec2(x2, y2);

				int tli = img->get_index(MathUtil::int_repeat((int)tl.x, w - 1), MathUtil::int_clamp((int)tl.y, h - 1));
				int tri = img->get_index(MathUtil::int_repeat((int)tr.x, w - 1), MathUtil::int_clamp((int)tr.y, h - 1));
				int bli = img->get_index(MathUtil::int_repeat((int)bl.x, w - 1), MathUtil::int_clamp((int)bl.y, h - 1));
				int bri = img->get_index(MathUtil::int_repeat((int)br.x, w - 1), MathUtil::int_clamp((int)br.y, h - 1));
				float tls = img->get_g_unsafe<uint16_t>(tli) / 65535.0f;
				float trs = img->get_g_unsafe<uint16_t>(tri) / 65535.0f;
				float bls = img->get_g_unsafe<uint16_t>(bli) / 65535.0f;
				float brs = img->get_g_unsafe<uint16_t>(bri) / 65535.0f;

				// Interpolate in X, which we use as the base interp
				float x_point = (cf.x - x1) / (x2 - x1);

				float xtop = glm::mix(tls, trs, x_point);
				float xbot = glm::mix(bls, brs, x_point);

				// Interpolate in Y, between xtop and xbot
				float y_point = (cf.y - y1) / (y2 - y1);
				float interp;

				if (x1 == x2 && y1 == y2)
				{
					// Return any of them
					interp = tls;
				}
				else if (x1 == x2)
				{
					// Interpolate in y
					interp = glm::mix(tls, bls, y_point);
				}
				else if (y1 == y2)
				{
					// Interpolate in x
					interp = glm::mix(tls, trs, x_point);
				}
				else
				{
					interp = glm::mix(xtop, xbot, y_point);
				}

				return interp;

			};

			// Raw pixel getter 
			out_image["get_pixel_raw"] = [&lua_state, img](int x, int y)
			{
				uint16_t v = img->get_g_unsafe<uint16_t>(img->get_index(x, y));

				return v;
			};

			out_image["get_height"] = [&lua_state, img](float x, float y)
			{
				x += glm::pi<float>();
				x /= glm::two_pi<float>();
				y /= glm::pi<float>();


				int w = img->get_width();
				int h = img->get_height();

				int ix = MathUtil::int_repeat((int)std::round(x * (float)w), w - 1);
				int iy = MathUtil::int_clamp((int)std::round(y * (float)h), h - 1);

				return (float)img->get_g_unsafe<uint16_t>(img->get_index(ix, iy)) / 65535.0f;

			};


			return out_image;
		};
	}
};