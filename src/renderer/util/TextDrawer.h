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
// For GUI code use NanoVG, this is meant to be used for 3D text
// TODO: Maybe NanoVG allows us to fully remove this class, think about it
// TODO: Add TrueType (FreeType?) font support
//
class TextDrawer
{
private:

	// We draw every letter with a single quad
	// If perfomance is bad I will write a new 
	// faster method (hopefully only 1 draw call per text)
	GLuint vao, vbo;

	Shader* shader;

	void draw_glyphs(const std::vector<BitmapFont::Glyph>& ps, BitmapFont* font, glm::vec2 pos, glm::ivec2 screen,
		glm::vec4 color, glm::vec2 scale);

public:

	enum Alignment
	{
		LEFT,
		CENTER,
		RIGHT
	};

	// String is encoded as UTF-8
	void draw_text(const std::string& text, BitmapFont* font, glm::vec2 pos, glm::ivec2 screen, 
		glm::vec4 color = glm::vec4(1.0, 1.0, 1.0, 1.0), glm::vec2 scale = glm::vec2(1.0f, 1.0f));

	void draw_text_aligned(const std::string& text, BitmapFont* font, glm::vec2 pos, Alignment alig, 
		glm::ivec2 screen, glm::vec4 color = glm::vec4(1.0, 1.0, 1.0, 1.0), glm::vec2 scale = glm::vec2(1.0f, 1.0f));

	// Gets the width of a given text in pixels (apply scale by multiplying)
	int get_size(const std::string& text, BitmapFont* font);

	TextDrawer();
	~TextDrawer();
};

extern TextDrawer* text_drawer;

void create_global_text_drawer();
void destroy_global_text_drawer();
