#pragma once
#include <unordered_map>
#include <glad/glad.h>

// Uses the AssetManager to obtain fonts!
class TextDrawer
{
public:


	TextDrawer();
	~TextDrawer();
};

extern TextDrawer* text_drawer;

void create_global_text_drawer();
void destroy_global_text_drawer();