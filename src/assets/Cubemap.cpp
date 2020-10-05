#include "Cubemap.h"
#include "Image.h"
#include <util/Logger.h>
#include "AssetManager.h"
#include <stb/stb_image.h>
#include <renderer/geometry/CubeGeometry.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

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

	std::string ext = images[0].substr(images[0].find_last_of('.'));
	bool is_hdr = ext == ".hdr";

	for(size_t side = 0; side < 6; side++)
	{
		int width, height, channels;
		if(is_hdr)
		{
			float* data = stbi_loadf(images[side].c_str(), &width, &height, &channels, 0);
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + side, 0, GL_RGB16F, width, height, 0, GL_RGB,
				GL_FLOAT, data);
		}
		else
		{
			uint8_t* data = stbi_load(images[side].c_str(), &width, &height, &channels, 0);
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

void Cubemap::generate_ibl_irradiance(size_t res, int face, bool bind)
{
	if(irradiance == nullptr)
	{
		irradiance = new Cubemap(res);
		irradiance_shader = AssetHandle<Shader>("core:shaders/ibl/irradiance.vs");
		glGenFramebuffers(1, &capture_fbo);
		glGenRenderbuffers(1, &capture_rbo);
		glBindFramebuffer(GL_FRAMEBUFFER, capture_fbo);
    	glBindRenderbuffer(GL_RENDERBUFFER, capture_rbo);
    	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, resolution, resolution);
    	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, capture_rbo);
    	CubeGeometry::generate_cubemap(&cubemap_vao, &cubemap_vbo);

		old_resolution = resolution;
	}

	logger->check(resolution == old_resolution, "Cannot change IBL resolution after it has been created");

	if(face < 0)
	{
		for(size_t side = 0; side < 6; side++)
		{
			generate_ibl_irradiance(res, side, false);
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

    GLint old_fbo, old_vport[4];
    if(face == 0 || bind)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, id);
		// We need to restore previous conditions, so store them
		glGetIntegerv(GL_FRAMEBUFFER_BINDING, &old_fbo);
		glGetIntegerv(GL_VIEWPORT, old_vport);

		glBindFramebuffer(GL_FRAMEBUFFER, capture_fbo);
		glViewport(0, 0, res, res);
	}

	irradiance_shader->use();
	irradiance_shader->setInt("tex", 0);
	irradiance_shader->setMat4("tform", proj * view[face]);


	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + face,
				irradiance->id, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glBindVertexArray(cubemap_vao);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);

    if(face == 5 || bind)
	{
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
		glBindFramebuffer(GL_FRAMEBUFFER, old_fbo);
		glViewport(old_vport[0], old_vport[1], old_vport[2], old_vport[3]);
	}
}

Cubemap::Cubemap(size_t nresolution)
{
	resolution = nresolution;

	glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_CUBE_MAP, id);

	for(size_t side = 0; side < 6; side++)
	{
		// We generate HDR by default, TODO: Give an option to toggle this behaviour
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + side, 0, GL_RGB16F, resolution, resolution, 0, GL_RGBA,
					 GL_UNSIGNED_BYTE, nullptr);
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
}

Cubemap::~Cubemap()
{
	if(irradiance != nullptr)
	{
		delete irradiance;
		glDeleteFramebuffers(1, &capture_fbo);
		glDeleteRenderbuffers(1, &capture_rbo);
		glDeleteVertexArrays(1, &cubemap_vao);
		glDeleteBuffers(1, &cubemap_vbo);
	}

	glDeleteTextures(1, &id);

}
