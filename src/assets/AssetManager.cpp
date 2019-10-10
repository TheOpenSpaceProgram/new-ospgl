#include "AssetManager.h"
#include "Shader.h"
#include <istream>
#include <fstream>

AssetManager* assets;


void createGlobalAssetManager()
{
	assets = new AssetManager();

	assets->createAssetType<Shader>(loadShader, "./res/shaders/", "vs", true);
	//assets->createAssetType<Texture>(loadTexture, "./res/tex/", "png", true);
}

void destroyGlobalAssetManager()
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
