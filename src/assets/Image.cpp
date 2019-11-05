#include "Image.h"
#include "AssetManager.h"
#include <stb/stb_image.h>
#include <sstream>

int Image::get_index(int x, int y)
{
	logger->check(x >= 0 && y >= 0 && x < width && y < height, "Pixel out of bounds");
	return y * width + x;
}

Image::Image(ImageConfig config, const std::string& path)
{
	this->config = config;

	u8data = nullptr;
	u16data = nullptr;
	fdata = nullptr;

	logger->check(config.channels == 1 || config.channels == 2 || 
		config.channels == 3 || config.channels == 4, "Invalid image channels");

	logger->check(config.bitdepth == 8 || config.bitdepth == 16 ||
		config.bitdepth == 32, "Invalid image bit depth");


	int c_dump;
	if (config.bitdepth == 8)
	{
		u8data = stbi_load(path.c_str(), &width, &height, &c_dump, config.channels);
	}
	else if (config.bitdepth == 16)
	{
		u16data = stbi_load_16(path.c_str(), &width, &height, &c_dump, config.channels);
	}
	else
	{
		fdata = stbi_loadf(path.c_str(), &width, &height, &c_dump, config.channels);
	}


	if (!config.in_memory)
	{
		if (u8data != nullptr)
		{
			delete u8data;
			u8data = nullptr;
		}
		if (u16data != nullptr)
		{
			delete u16data;
			u16data = nullptr;
		}
		if (fdata != nullptr)
		{
			delete fdata;
			fdata = nullptr;
		}
	}

	if (config.upload)
	{
		// TODO
	}
}


Image::~Image()
{
}

static const std::string default_toml = R"-(

bitdepth = 8
channels = 4
upload = true
in_memory = false

)-";

Image* loadImage(const std::string& path)
{
	if (!AssetManager::fileExists(path))
	{
		logger->error("Could not load image {}, file not found!", path);
		return nullptr;
	}

	// Find TOML config file in same path if present,
	// otherwise use default config
	// We get the path to the vertex shader
	std::string tomlpath = path.substr(0, path.find_last_of('.')) + ".toml";

	std::string toml = default_toml;
	std::stringstream tomls = std::stringstream(toml);

	bool has_toml_file = false;
	if (AssetManager::fileExists(tomlpath))
	{
		toml = assets->loadString(tomlpath);
		has_toml_file = true;
	}



	if (!has_toml_file)
	{
		logger->info("Loading image from file: '{}'", path);
	}
	else
	{
		logger->info("Loading image from file: '{}' with config", path);
	}

	ImageConfig config;
	
	auto tomlv = SerializeUtil::load_string(toml);
	::deserialize(config, *tomlv);
	
	Image* n_image = new Image(config, path);

	return n_image;
}
