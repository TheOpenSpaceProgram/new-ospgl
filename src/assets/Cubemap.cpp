#include "Cubemap.h"
#include "Image.h"
#include <util/Logger.h>
#include "AssetManager.h"
#include <stb/stb_image.h>

Cubemap* load_cubemap(const std::string& path, const std::string& name,
					  const std::string& pkg, const cpptoml::table& cfg)
{
	size_t last_dot = path.find_last_of('.');
	if(last_dot == std::string::npos)
	{
		logger->error("Cubemap '{}' must be stored in a folder with a file extension", path);
		return nullptr;
	}

	std::string subname = path.substr(0, last_dot);
	std::string extension = path.substr(last_dot);
	const std::string sides[6] = {"px", "nx", "py", "ny", "pz", "nz"};
	std::vector<std::string> images;
	images.resize(6);
	for(size_t side = 0; side < 6; side++)
	{
		std::string subpath = subname; subpath += extension; subpath += "/";
		subpath += sides[side]; subpath += extension;
		images[side] = subpath;
	}

	// Pass everything to cubemap
	return new Cubemap(images);
}

Cubemap::Cubemap(std::vector<std::string> &images) : id(0)
{
	glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_CUBE_MAP, id);

	for(size_t side = 0; side < 6; side++)
	{
		int width, height, channels;
		uint8_t* data = stbi_load(images[side].c_str(), &width, &height, &channels, 0);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + side, 0, GL_RGB, width, height, 0, GL_RGBA,
			   GL_UNSIGNED_BYTE, data);

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	}

}
