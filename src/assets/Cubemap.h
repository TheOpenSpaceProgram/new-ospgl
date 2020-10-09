#pragma once
#include <glad/glad.h>
#include <string>
#include <cpptoml.h>
#include "Shader.h"
#include "Image.h"
#include "AssetManager.h"

// Cubemaps are used for a variety of effects, including skyboxes
// and PBR irradiance maps (realtime generated or pregenerated)
// These are OpenGL only, they could also bmagee accesible from RAM, though
// These are loaded from a folder with images named:
// px, py, pz, nx, ny, nz [. folder extension]
class Cubemap
{
private:

	AssetHandle<Shader> irradiance_shader;
	AssetHandle<Shader> specular_shader;
	AssetHandle<Image> brdf_lut;
	GLuint capture_fbo = 0, capture_rbo = 0;
	GLuint cubemap_vao = 0, cubemap_vbo = 0;
	size_t old_resolution = 0;

public:

	size_t resolution = 0;
	GLuint id = 0;

	// nullptr if not used for IBL
	Cubemap* irradiance = nullptr;
	Cubemap* specular = nullptr;

	// If face is < 0 then all faces are generated
	void generate_ibl_irradiance(size_t res = 32, size_t spec_res = 128, int face = -1, bool bind = true);


	Cubemap(std::vector<std::string>& images);
	// Creates empty textures, to be used as rendertargets
	Cubemap(size_t nresolution, bool mipmap = false);
	~Cubemap();

};

Cubemap* load_cubemap(const std::string& path, const std::string& name, const std::string& pkg, const cpptoml::table& cfg);
