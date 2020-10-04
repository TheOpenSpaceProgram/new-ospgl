#pragma once
#include <glad/glad.h>
#include <string>
#include <cpptoml.h>

template<typename T>
class AssetHandle;

// Cubemaps are used for a variety of effects, including skyboxes
// and PBR irradiance maps (realtime generated or pregenerated)
// These are OpenGL only, they could also bmagee accesible from RAM, though
// These are loaded from a folder with images named:
// px, py, pz, nx, ny, nz [. folder extension]
class Cubemap
{
public:

	GLuint id;

	Cubemap(std::vector<std::string>& images);

};

Cubemap* load_cubemap(const std::string& path, const std::string& name, const std::string& pkg, const cpptoml::table& cfg);
