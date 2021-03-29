#pragma once
#include <glad/glad.h>
#include <string>
#include <cpptoml.h>
#include "Shader.h"
#include "Image.h"
#include "AssetManager.h"
#include "Asset.h"

// Cubemaps are used for a variety of effects, including skyboxes
// and PBR irradiance maps (realtime generated or pregenerated)
// These are OpenGL only, they could also bmagee accesible from RAM, though
// These are loaded from a folder with images named:
// px, py, pz, nx, ny, nz [. folder extension]
class Cubemap : public Asset
{
private:

	AssetHandle<Shader> irradiance_shader;
	AssetHandle<Shader> specular_shader;
	AssetHandle<Shader> blit_shader;
	AssetHandle<Image> brdf_lut;
	GLuint capture_fbo = 0, capture_rbo = 0;
	GLuint cubemap_vao = 0, cubemap_vbo = 0;
	size_t old_resolution = 0;

	static const size_t MAX_MIP = 5;
	static const size_t SAMPLE_COUNT = 32;

	std::vector<float> pcomp_mip_levels;

	GLuint pcomp_sample_dir_tex[MAX_MIP - 1];

	GLint old_fbo, old_vport[4];

public:

	size_t resolution = 0;
	GLuint id = 0;

	// nullptr if not used for IBL
	Cubemap* irradiance = nullptr;
	Cubemap* specular = nullptr;

	// If face is < 0 then all faces are generated
	void generate_ibl_irradiance(size_t res = 32, size_t spec_res = 128, int face = -1, bool bind = true);


	Cubemap(std::vector<std::string>& images, ASSET_INFO);
	// Creates empty textures, to be used as rendertargets
	Cubemap(size_t nresolution, bool mipmap = false);
	~Cubemap();

};

Cubemap* load_cubemap(ASSET_INFO, const cpptoml::table& cfg);
