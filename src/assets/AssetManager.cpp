#include "AssetManager.h"
#include "Shader.h"
#include <istream>
#include <fstream>

AssetManager* assets;


void create_global_asset_manager()
{
	assets = new AssetManager();

	assets->createAssetType<Shader>(loadShader, "./res/shaders/", "vs", true);
	//assets->createAssetType<Texture>(loadTexture, "./res/tex/", "png", true);
}

void destroy_global_asset_manager()
{
	delete assets;
}

std::string AssetManager::loadString(const std::string& path)
{
	std::ifstream t(path);
	std::string str((std::istreambuf_iterator<char>(t)),
		std::istreambuf_iterator<char>());
	return str;
}
