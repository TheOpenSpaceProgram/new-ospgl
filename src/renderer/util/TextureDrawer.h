#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "../../assets/Shader.h"

// A simple, static class to aid drawing simple 2D textures
// (including framebuffers) to the screen
class TextureDrawer
{
private:

	GLuint vao, vbo;
	GLuint full_vao, full_vbo;

	Shader* shader;

public:

	// All coordinates are pixel screen coordinates
	// (We use ortographic projection)
	void draw(GLuint tex, glm::vec2 pos, glm::vec2 size, glm::ivec2 screen_size, bool vflip = true);
	void draw(GLuint tex, glm::vec2 pos, glm::vec2 size, glm::ivec2 screen_size, Shader* cshader, bool vflip = true);
	void issue_fullscreen_rectangle();
	void issue_rectangle(glm::vec2 pos0, glm::vec2 size);

	TextureDrawer();
	~TextureDrawer();
};

extern TextureDrawer* texture_drawer;

void create_global_texture_drawer();
void destroy_global_texture_drawer();
