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
	// Position (alpha is emissive), normal and albedo (rgb) + specular (alpha)
	GLuint g_pos, g_nrm, g_col;
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

