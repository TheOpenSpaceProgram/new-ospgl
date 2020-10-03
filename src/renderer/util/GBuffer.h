#pragma once 

#include <glad/glad.h>
#include <glm/glm.hpp>

// Basically a normal framebuffer but with more... buffers
// Used by the deferred renderer
class GBuffer
{
private:

	size_t width, height;

	GLint unbind_to;

	GLsizei unbind_viewport[4];

public:

	GLuint g_buffer;
	// pos:
	// - R, G, B = Position
	// - A = Emissive factor (albedo is used as color) (*)
	// nrm:
	// - R, G, B = Normal
	// col:
	// - R, G, B = Albedo
	// pbr: (Designed to conform more or less to gltf)
	// - R 		 = Ambient Occlussion
	// - G 		 = Rougness
	// - B		 = Metalness
	// (*) Emissive is in pos as RGBA_32F is widely supported, but RGB_32F is not!
	GLuint g_pos, g_nrm, g_col, g_pbr;
	// Depth buffer
	GLuint rbo;

	void create(size_t width, size_t height);
	void destroy();

	void bind();

	void unbind();

	void set_viewport();

	glm::ivec2 get_size();

	GBuffer(size_t width, size_t height);
	~GBuffer();
};

