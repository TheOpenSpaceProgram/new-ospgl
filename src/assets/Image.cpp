#include "Image.h"
#include "AssetManager.h"
#include <stb/stb_image.h>
#include <sstream>
#include "../util/MathUtil.h"

int Image::get_index(int x, int y)
{
	logger->check(x >= 0 && y >= 0 && x < width && y < height, 
			"Pixel out of bounds ({},{}) / ({},{})", x, y, width, height);
	return y * width + x;
}

glm::vec4 Image::sample_bilinear(float x, float y)
{
	logger->check(config.in_memory, "Image must be present in RAM for CPU sampling");

	int w = get_width();
	int h = get_height();

	glm::vec2 cf = glm::vec2(x * w, y * h);

	float x1 = floor(x * (float)w);
	float x2 = ceil(x * (float)w);
	float y1 = floor(y * (float)h);
	float y2 = ceil(y * (float)h);



	glm::vec2 tl = glm::vec2(x1, y1);
	glm::vec2 tr = glm::vec2(x2, y1);
	glm::vec2 bl = glm::vec2(x1, y2);
	glm::vec2 br = glm::vec2(x2, y2);

	int tli = get_index(MathUtil::int_repeat((int)tl.x, w - 1), MathUtil::int_clamp((int)tl.y, h - 1));
	int tri = get_index(MathUtil::int_repeat((int)tr.x, w - 1), MathUtil::int_clamp((int)tr.y, h - 1));
	int bli = get_index(MathUtil::int_repeat((int)bl.x, w - 1), MathUtil::int_clamp((int)bl.y, h - 1));
	int bri = get_index(MathUtil::int_repeat((int)br.x, w - 1), MathUtil::int_clamp((int)br.y, h - 1));

	glm::vec4 tls = get_rgba(tli);
	glm::vec4 trs = get_rgba(tri);
	glm::vec4 bls = get_rgba(bli);
	glm::vec4 brs = get_rgba(bri);

	// Interpolate in X, which we use as the base interp
	float x_point = (cf.x - x1) / (x2 - x1);

	glm::vec4 xtop = glm::mix(tls, trs, x_point);
	glm::vec4 xbot = glm::mix(bls, brs, x_point);

	// Interpolate in Y, between xtop and xbot
	float y_point = (cf.y - y1) / (y2 - y1);
	glm::vec4 interp;

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
}

glm::vec4 Image::sample_bilinear(glm::vec2 px)
{
	return sample_bilinear(px.x, px.y);
}

glm::dvec4 Image::sample_bilinear_double(glm::dvec2 px)
{
	return sample_bilinear_double(px.x, px.y);
}

glm::dvec4 Image::sample_bilinear_double(double x, double y)
{
	return sample_bilinear((float)x, (float)y);
}

glm::vec4 Image::get_rgba(int i)
{
	return glm::vec4(fdata[i * 4 + 0], fdata[i * 4 + 1], fdata[i * 4 + 2], fdata[i * 4 + 3]);
}

glm::vec4 Image::get_rgba(int x, int y)
{
	return get_rgba(get_index(x, y));
}

Image::Image(ImageConfig config, const std::string& path)
{
	this->config = config;

	int c_dump;

	uint8_t* u8data;

	if(config.is_font)
	{
		uint8_t* font_u8data = stbi_load(path.c_str(), &width, &height, &c_dump, 1);
		// We now expand the buffer to full size RGBA, using alpha for blending
		// TODO:
		//  This is sub-optimal for memory, and it could be better to simply store
		//  the texture as a 1-channel texture and modify NanoVG to handle it, but 
		//  the perfomance hit is tiny
		u8data = (uint8_t*)malloc(width * height * 4);
		for(size_t i = 0; i < width * height; i++)
		{
			uint8_t val = font_u8data[i];
			uint8_t rgb = val == 0 ? 0 : 255;
			u8data[i * 4 + 0] = rgb;
			u8data[i * 4 + 1] = rgb;
			u8data[i * 4 + 2] = rgb;
			u8data[i * 4 + 3] = val; // < Alpha
		}

	}
	else
	{
		u8data = stbi_load(path.c_str(), &width, &height, &c_dump, 4);
		fdata = nullptr;
	}

	if (config.upload)
	{
		logger->debug("Uploading texture to OpenGL");

		glGenTextures(1, &id);
		glBindTexture(GL_TEXTURE_2D, id); // all upcoming GL_TEXTURE_2D operations now have effect on this texture object
		// set the texture wrapping parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		// set texture filtering parameters
		if(config.filter == ImageConfig::NEAREST)
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		}
		else
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		}

		GLenum source_format;
		source_format = GL_RGBA;

		GLenum target_format = GL_RGBA;

		glTexImage2D(GL_TEXTURE_2D, 0, target_format, width, height, 0, source_format, GL_UNSIGNED_BYTE, u8data);

		
		glGenerateMipmap(GL_TEXTURE_2D);
	}

	if (config.in_memory)
	{
		// We must convert image data to floats
		fdata = (float*)malloc(width * height * 4 * sizeof(float));
		for (size_t i = 0; i < width * height; i++)
		{
			fdata[i * 4 + 0] = (float)u8data[i * 4 + 0] / 255.0f;
			fdata[i * 4 + 1] = (float)u8data[i * 4 + 1] / 255.0f;
			fdata[i * 4 + 2] = (float)u8data[i * 4 + 2] / 255.0f;
			fdata[i * 4 + 3] = (float)u8data[i * 4 + 3] / 255.0f;
		}
	}

	if(config.is_font)
	{
		free(u8data);
	}
	else
	{
		stbi_image_free(u8data);
	}

}


Image::~Image()
{
	if (config.in_memory)
	{
		if (fdata != nullptr)
		{
			free(fdata);
			fdata = nullptr;
		}
	}

	if (config.upload && id != 0)
	{
		glDeleteTextures(1, &id);
	}
}

static const std::string default_toml = R"-(

upload = true
in_memory = false

)-";

Image* load_image(const std::string& path, const std::string& name, const std::string& pkg, const cpptoml::table& cfg)
{
	if (!AssetManager::file_exists(path))
	{
		logger->error("Could not load image {}, file not found!", path);
		return nullptr;
	}

	std::shared_ptr<cpptoml::table> def_toml = SerializeUtil::load_string(default_toml);


	std::shared_ptr<cpptoml::table> sub_ptr = nullptr;

	if (cfg.get_table_qualified("image"))
	{
		sub_ptr = cfg.get_table_qualified("image");
	}
	else
	{
		sub_ptr = def_toml;
	}

	ImageConfig config;
	::deserialize(config, *sub_ptr);

	Image* n_image = new Image(config, path);

	return n_image;
}
