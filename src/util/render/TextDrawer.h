#pragma once
#include <unordered_map>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "../../assets/BitmapFont.h"
#include "../../assets/Shader.h"

// Uses the AssetManager to obtain fonts dinamically.
//
// Bitmap fonts use https://www.angelcode.com/products/bmfont/ format
// Freetype fonts are yet to be implemented
//

class TextDrawer
{
private:

	// We draw every letter with a single quad
	// If perfomance is bad I will write a new 
	// faster method (hopefully only 1 draw call per text)
	GLuint vao, vbo;

	Shader* shader;

public:

	// String is encoded as UTF-8
	void draw_text(const std::string& text, BitmapFont* font, glm::vec2 pos, glm::ivec2 screen, 
		glm::vec4 color = glm::vec4(1.0, 1.0, 1.0, 1.0), float scale = 1.0f);

	TextDrawer();
	~TextDrawer();
};

extern TextDrawer* text_drawer;

void create_global_text_drawer();
void destroy_global_text_drawer();