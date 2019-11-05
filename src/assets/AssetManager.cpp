#include "AssetManager.h"
#include "Shader.h"
#include "Image.h"
#include <istream>
#include <fstream>

AssetManager* assets;


void create_global_asset_manager()
{
	assets = new AssetManager();

	assets->createAssetType<Shader>(loadShader, "./res/shaders/", "vs", true);
	assets->createAssetType<Image>(loadImage, "./res/", "png", true);
}

void destroy_global_asset_manager()
{
	delete assets;
}

bool AssetManager::fileExists(const std::string& path)
{
	std::ifstream f(path.c_str());
	return f.good();
}

std::string AssetManager::loadString(const std::string& path)
{
	std::ifstream t(path);
	std::string str((std::istreambuf_iterator<char>(t)),
		std::istreambuf_iterator<char>());
	return str;
}
