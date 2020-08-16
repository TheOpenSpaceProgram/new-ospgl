#pragma once
#include <string>
#include <cpptoml.h>
#include <unordered_map>
#include "Image.h"

// Loads a font as exported from BMFont
// and parses its glyphs. 
// The texture file is loaded automatically
// BMFont settings:
// Binary export format
// Only supports one page
// Font file must be .png and named the same as the .fnt file
// (except for extension)

class BitmapFont
{
public:

	Image* img;

	// The number of pixels from the absolute top of the line to the base of the characters.
	int base;

	// The number of pixels from one line to another (y)
	int line_height;

	struct Glyph
	{
		// Position of the glyph in the img
		uint16_t x, y;

		// Size of the glyph in the img
		uint16_t width, height;

		// Offset to apply when copying from img to screen
		int xoffset, yoffset;

		// How much to move the cursor after drawing the character
		int xadvance;

	};

	std::unordered_map<uint32_t, Glyph> chars;

	BitmapFont(const std::vector<uint8_t>& fnt, Image* image);
	~BitmapFont();
};

BitmapFont* load_bitmap_font(const std::string& path, const std::string& name, const std::string& pkg, const cpptoml::table& cfg);


