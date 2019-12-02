#include "BitmapFont.h"
#include "AssetManager.h"
#include <iostream>
#include <fstream>
#include "../util/render/TextDrawer.h"

// TODO: Not really sure about endianness on this thing?

struct Block 
{
	char id;
	uint32_t size;
	// Includes size and id bytes
	uint32_t real_size;

	size_t start_ptr;
};

uint32_t read_uint32_t(size_t offset, const std::vector<uint8_t>& fnt)
{
	uint32_t o0 = (uint32_t)fnt[offset + 0];
	uint32_t o1 = (uint32_t)fnt[offset + 1];
	uint32_t o2 = (uint32_t)fnt[offset + 2];
	uint32_t o3 = (uint32_t)fnt[offset + 3];

	return o3 << 24 | o2 << 16 | o1 << 8 | o0;
}

uint16_t read_uint16_t(size_t offset, const std::vector<uint8_t>& fnt)
{
	uint16_t o0 = (uint16_t)fnt[offset + 0];
	uint16_t o1 = (uint16_t)fnt[offset + 1];

	return o1 << 8 | o0;
}

int read_int(size_t offset, const std::vector<uint8_t>& fnt)
{
	uint8_t o0 = fnt[offset + 0];
	uint8_t o1 = fnt[offset + 1];

	// Some weird stuff has to be done to keep bits unperturbed

	uint16_t lower = o1 << 8 | o0;

	int16_t as_int16 = static_cast<int16_t>(lower);

	return (int)as_int16;
}

Block get_block(size_t offset, const std::vector<uint8_t>& fnt)
{
	Block out;
	
	out.id = fnt[offset];

	out.size = read_uint32_t(offset + 1, fnt);
	out.real_size = out.size + 5;

	out.start_ptr = offset + 5;

	return out;
}


void BitmapFont::draw_text(const std::string& text, glm::vec2 pos, glm::ivec2 screen, glm::vec4 color, float scale)
{
	text_drawer->draw_text(text, this, pos, screen, color, scale);
}

BitmapFont::BitmapFont(const std::vector<uint8_t>& fnt, Image* image)
{
	this->img = image;


	logger->check(fnt[0] == 66 && fnt[1] == 77 && fnt[2] == 70 && fnt[3] == 3, "Invalid font file");

	size_t ptr = 4;

	// Extract all blocks
	std::vector<Block> blocks;
	while (ptr < fnt.size())
	{
		blocks.push_back(get_block(ptr, fnt));
		ptr += blocks[blocks.size() - 1].real_size;
	}

	// Find blocks
	for (size_t i = 0; i < blocks.size(); i++)
	{
		if (blocks[i].id == 2)
		{
			// Common
			line_height = (int)read_uint16_t(blocks[i].start_ptr, fnt);
			base = (int)read_uint16_t(blocks[i].start_ptr + 2, fnt);
		}
		else if (blocks[i].id == 4)
		{
			// Chars
			for (size_t c = 0; c < blocks[i].size / 20; c++)
			{
				size_t char_ptr = c * 20 + blocks[i].start_ptr;
				Glyph g;

				uint32_t id = read_uint32_t(char_ptr + 0, fnt);
				g.x = read_uint16_t(char_ptr + 4, fnt);
				g.y = read_uint16_t(char_ptr + 6, fnt);
				g.width = read_uint16_t(char_ptr + 8, fnt);
				g.height = read_uint16_t(char_ptr + 10, fnt);
				g.xoffset = read_int(char_ptr + 12, fnt);
				g.yoffset = read_int(char_ptr + 14, fnt);
				g.xadvance = read_int(char_ptr + 16, fnt);

				chars[id] = g;
			}
		}
		// TODO: Parse kerning (maybe neccesary for some fonts)
	}

	logger->debug("Loaded {} characters", chars.size());
}


BitmapFont::~BitmapFont()
{
}

BitmapFont* loadBitmapFont(const std::string& path, const std::string& name, const std::string& pkg, const cpptoml::table& cfg)
{
	std::vector<uint8_t> font = assets->load_binary_raw(path);

	Image* font_image = assets->get<Image>(pkg, name.substr(0, name.find_last_of('.')) + ".png");

	return new BitmapFont(font, font_image);
}
