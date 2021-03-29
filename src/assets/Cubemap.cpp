#include "Cubemap.h"
#include "Image.h"
#include <util/Logger.h>
#include "AssetManager.h"
#include <stb/stb_image.h>
#include <renderer/geometry/CubeGeometry.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

Cubemap* load_cubemap(ASSET_INFO, const cpptoml::table& cfg)
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
	return new Cubemap(images, ASSET_INFO_P);
}

Cubemap::Cubemap(std::vector<std::string> &images, ASSET_INFO) : id(0), Asset(ASSET_INFO_P)
{
	glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_CUBE_MAP, id);

	std::string ext = images[0].substr(images[0].find_last_of('.'));
	bool is_hdr = ext == ".hdr";

	for(size_t side = 0; side < 6; side++)
	{
		int width, height, channels;
		if(is_hdr)
		{
			float* data = stbi_loadf(images[side].c_str(), &width, &height, &channels, 3);
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + side, 0, GL_RGB16F, width, height, 0, GL_RGB,
				GL_FLOAT, data);
		}
		else
		{
			uint8_t* data = stbi_load(images[side].c_str(), &width, &height, &channels, 4);
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + side, 0, GL_RGB, width, height, 0, GL_RGBA,
						 GL_UNSIGNED_BYTE, data);
		}
		logger->check(width == height, "Cubemap faces must be squares");
		resolution = width;
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

}

void Cubemap::generate_ibl_irradiance(size_t res, size_t spec_res, int face, bool bind)
{

	if(irradiance == nullptr)
	{
		irradiance = new Cubemap(res);
		specular = new Cubemap(spec_res, true);
		irradiance_shader = AssetHandle<Shader>("core:shaders/ibl/irradiance.vs");
		specular_shader = AssetHandle<Shader>("core:shaders/ibl/specular.vs");
		blit_shader = AssetHandle<Shader>("core:shaders/skybox.vs");
		brdf_lut = AssetHandle<Image>("core:shaders/ibl/brdf.png");
		glGenFramebuffers(1, &capture_fbo);
		glGenRenderbuffers(1, &capture_rbo);
		glBindFramebuffer(GL_FRAMEBUFFER, capture_fbo);
    	glBindRenderbuffer(GL_RENDERBUFFER, capture_rbo);
    	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, capture_rbo);

    	CubeGeometry::generate_cubemap(&cubemap_vao, &cubemap_vbo);

		old_resolution = resolution;

		// Precompute the sample directions (this also prevents jitter)
		// (Copyed from the shader)
		std::array<glm::vec3, SAMPLE_COUNT> sample_dirs;
		for(size_t mip = 1; mip < MAX_MIP; mip++)
		{
			float roughness = (float) mip / (float) (MAX_MIP - 1);
			for (uint32_t i = 0; i < SAMPLE_COUNT; i++)
			{
				// Radical_Inverse_VdC
				uint32_t bits = (i << 16u) | (i >> 16u);
				bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
				bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
				bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
				bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
				float VdC = float(bits) * 2.3283064365386963e-10f;

				// Hammersley
				glm::vec2 Xi = glm::vec2(float(i)/float(SAMPLE_COUNT), VdC);

				// ImportanceSampleGGX
				float a = roughness * roughness;
				float phi = 2.0f * glm::pi<float>() * Xi.x;
				float cosTheta = glm::sqrt((1.0f - Xi.y) / (1.0f + (a*a - 1.0f) * Xi.y));
				float sinTheta = glm::sqrt(1.0f - cosTheta*cosTheta);

				// from spherical coordinates to cartesian coordinates
				glm::vec3 H;
				H.x = glm::cos(phi) * sinTheta;
				H.y = glm::sin(phi) * sinTheta;
				H.z = cosTheta;

				// TODO: Investigate a better way to do this without
				// losing too much precision. This method may be too loosy!
				// It causes small discrepancy of about 0.00393
				// (We could use a non-normalized format? Investigate a bit)
				sample_dirs[i] = H * 0.5f + 0.5f;

			}

			// Upload it to a texture
			glGenTextures(1, &pcomp_sample_dir_tex[mip - 1]);
			glBindTexture(GL_TEXTURE_1D, pcomp_sample_dir_tex[mip - 1]);
			glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB, SAMPLE_COUNT, 0, GL_RGB, GL_FLOAT, (float*)sample_dirs.data());
			glGenerateMipmap(GL_TEXTURE_1D);
			glBindTexture(GL_TEXTURE_1D, 0);
		}

	}

	logger->check(resolution == old_resolution, "Cannot change IBL resolution after it has been created");

	if(face < 0)
	{
		for(size_t side = 0; side < 6; side++)
		{
			generate_ibl_irradiance(res, spec_res, side, false);
		}

		return;
	}

	// Copyed from learnopengl.com
	glm::mat4 proj = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
    glm::mat4 view[] =
    {
        glm::lookAt(glm::vec3(0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
        glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
        glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
    };

    if(face == 0 || bind)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, id);
		// We need to restore previous conditions, so store them
		glGetIntegerv(GL_FRAMEBUFFER_BINDING, &old_fbo);
		glGetIntegerv(GL_VIEWPORT, old_vport);

		glBindFramebuffer(GL_FRAMEBUFFER, capture_fbo);
	}

	glBindRenderbuffer(GL_RENDERBUFFER, capture_rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, resolution, resolution);
	glViewport(0, 0, res, res);
	irradiance_shader->use();
	irradiance_shader->setInt("tex", 0);
	irradiance_shader->setMat4("tform", proj * view[face]);


	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + face,
				irradiance->id, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDepthFunc(GL_LESS);

	glBindVertexArray(cubemap_vao);
    glDrawArrays(GL_TRIANGLES, 0, 36);

	// Specular pass
	specular_shader->use();
 	specular_shader->setInt("tex", 0);
 	specular_shader->setFloat("tex_size", this->resolution);
 	specular_shader->setMat4("tform", proj * view[face]);


	 for(size_t mip = 0; mip < 5; mip++)
	 {
		 size_t msize = spec_res * glm::pow(0.5f, mip);
		 glViewport(0, 0, msize, msize);
		 glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, msize, msize);

		 float roughness = (float)mip / (float)(MAX_MIP - 1);
		 specular_shader->setFloat("roughness", roughness);
		 if(mip != 0)
		 {
			 glActiveTexture(GL_TEXTURE1);
			 glBindTexture(GL_TEXTURE_1D, pcomp_sample_dir_tex[mip - 1]);
			 specular_shader->setInt("sample_positions", 1);
		 }
		 glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
						  GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, specular->id, mip);

		 glClear(GL_DEPTH_BUFFER_BIT);

		 glDrawArrays(GL_TRIANGLES, 0, 36);
	 }


    glBindVertexArray(0);

    if(face == 5 || bind)
	{
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
		glBindFramebuffer(GL_FRAMEBUFFER, old_fbo);
		glViewport(old_vport[0], old_vport[1], old_vport[2], old_vport[3]);
	}
}

Cubemap::Cubemap(size_t nresolution, bool mipmap) : Asset(GENERATED_ASSET_INFO)
{
	resolution = nresolution;

	glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_CUBE_MAP, id);

	for(size_t side = 0; side < 6; side++)
	{
		// We generate HDR by default, TODO: Give an option to toggle this behaviour
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + side, 0, GL_RGBA16F, resolution, resolution, 0, GL_RGBA,
					 GL_UNSIGNED_BYTE, nullptr);
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	if(mipmap)
	{
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
	}
	else
	{
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
}

Cubemap::~Cubemap()
{
	if(irradiance != nullptr)
	{
		delete irradiance;
		delete specular;
		glDeleteFramebuffers(1, &capture_fbo);
		glDeleteRenderbuffers(1, &capture_rbo);
		glDeleteVertexArrays(1, &cubemap_vao);
		glDeleteBuffers(1, &cubemap_vbo);
	}

	glDeleteTextures(1, &id);

}
