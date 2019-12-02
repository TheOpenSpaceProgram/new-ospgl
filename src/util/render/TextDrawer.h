#pragma once
#include <unordered_map>
#include <glad/glad.h>
#include <glm/glm.hpp>

// Uses the AssetManager to obtain fonts dinamically.
//
// Bitmap fonts use https://www.angelcode.com/products/bmfont/ format
// Freetype fonts are yet to be implemented
//

class TextDrawer
{
private:



public:

	void draw_bitmap_text(const std::wstring& text, glm::vec2 pos, glm::ivec2 screen, glm::vec3 color, float scale = 1.0f);

	TextDrawer();
	~TextDrawer();
};

extern TextDrawer* text_drawer;

void create_global_text_drawer();
void destroy_global_text_drawer();