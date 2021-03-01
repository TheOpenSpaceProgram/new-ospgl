#pragma once
#include <glad/glad.h>
#include <string>
#include <util/Logger.h>
#include <util/SerializeUtil.h>
#include "Asset.h"

struct ImageConfig
{
	enum FilterMode
	{
		NEAREST,
		LINEAR
	};

	bool upload;
	bool in_memory;
	// This converts the image from 1 channel to 4 with alpha for 
	// compatibility with NanoVG
	bool is_font;
	// This applies linear correction to the image
	bool is_srgb;
	FilterMode filter;
};

struct NVGcontext;

// Images are always RGBA, stored as float, for perfomance reasons
class Image : public Asset
{
private:

	float* fdata;

	ImageConfig config;

	int width, height;

	// TODO: If at any time there are more than 1 NVGcontexts, then we need
	// to write some kind of code here to handle multiple contexts
	int nanovg_image;
	NVGcontext* in_vg;

public:

	// This automatically creates the nanoVG image ONLY once
	int get_nvg_image(NVGcontext* vg);

	ImageConfig get_config() const { return config; }
	inline int get_width() const { return width; }
	inline int get_height() const { return height; }

	inline glm::ivec2 get_size() const { return glm::ivec2(width, height); }

	inline glm::dvec2 get_sized() const { return glm::dvec2(width, height); }

	int get_index(int x, int y);

	// Does sampling using bilinear interpolation
	// Coordinates should go from 0 to 1
	glm::vec4 sample_bilinear(float x, float y);
	glm::vec4 sample_bilinear(glm::vec2 px);

	// Functions for lua, they simply cast to floats
	glm::dvec4 sample_bilinear_double(glm::dvec2 px);
	glm::dvec4 sample_bilinear_double(double x, double y);

	// Safe function, works correctly across image types
	glm::vec4 get_rgba(int i);
	glm::vec4 get_rgba(int x, int y);

	GLuint id;

	Image(ImageConfig config, ASSET_INFO);
	Image(const unsigned char* data, int width, int height, int bits, int component,
	   int mag_filter, int min_filter, int wrapS, int wrapT, bool srgb, ASSET_INFO);
	~Image();
};

Image* load_image(ASSET_INFO, const cpptoml::table& cfg);



template<>
class GenericSerializer<ImageConfig>
{
public:

	static void serialize(const ImageConfig& what, cpptoml::table& target)
	{
		target.insert("upload", what.upload);
		target.insert("in_memory", what.in_memory);
		target.insert("is_font", what.is_font);
		target.insert("is_srgb", what.is_srgb);
		std::string filter_str = "linear";
		if(what.filter == ImageConfig::NEAREST)
		{
			filter_str = "nearest";
		}
		target.insert("filter", filter_str);
	}

	static void deserialize(ImageConfig& to, const cpptoml::table& from)
	{
		SAFE_TOML_GET_OR(to.upload, "upload", bool, true);
		SAFE_TOML_GET_OR(to.in_memory, "in_memory", bool, false);
		SAFE_TOML_GET_OR(to.is_font, "is_font", bool, false);
		SAFE_TOML_GET_OR(to.is_srgb, "is_srgb", bool, false);
		std::string filter_str;
		SAFE_TOML_GET_OR(filter_str, "filter", std::string, "linear");

		if(filter_str == "linear")
		{
			to.filter = ImageConfig::LINEAR;
		}
		else
		{
			to.filter = ImageConfig::NEAREST;
		}
	}
};
