#include "TextDrawer.h"
#include <codecvt>
#include <utf8/utf8.h>
#include "../../assets/AssetManager.h"

TextDrawer* text_drawer;

void TextDrawer::draw_glyphs(const std::vector<BitmapFont::Glyph>& ps, BitmapFont * font, glm::vec2 pos, 
	glm::ivec2 screen, glm::vec4 color, glm::vec2 scale)
{
	glDisable(GL_DEPTH_TEST);
	glm::vec3 spos = glm::vec3(pos.x, pos.y, 0.0f);
	glm::mat4 view = glm::ortho(0.0f, (float)screen.x, (float)screen.y, 0.0f, -1.0f, 1.0f);

	shader->use();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, font->img->id);
	glBindVertexArray(vao);

	shader->setInt("tex", 0);
	shader->setVec4("color", color);

	float iwidth = (float)font->img->get_width();
	float iheight = (float)font->img->get_height();

	for(size_t i = 0; i < ps.size(); i++)
	{
		BitmapFont::Glyph gl = ps[i];


		glm::vec3 gscale = glm::vec3((float)gl.width * glm::abs(scale.x), (float)gl.height * glm::abs(scale.y), 1.0f);

		glm::vec2 offset = glm::vec2((float)gl.xoffset * scale.x, (float)gl.yoffset * scale.y);

		glm::mat4 model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(offset, 0.0f));
		model = glm::translate(model, spos);
		model = glm::scale(model, gscale);
		glm::mat4 tform = view * model;

		glm::vec2 tex_off = glm::vec2((float)gl.x / iwidth, (float)gl.y / iheight);
		glm::vec2 tex_size = glm::vec2((float)gl.width / iwidth, (float)gl.height / iheight);


		shader->setMat4("tform", tform);
		shader->setVec4("texoff", glm::vec4(tex_off.x, tex_off.y, tex_size.x, tex_size.y));
		shader->setVec2("scale", scale);

		glDrawArrays(GL_TRIANGLES, 0, 6);
		spos.x += (float)gl.xadvance * scale.x;

	}

	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);

	glEnable(GL_DEPTH_TEST);
}

std::pair<std::vector<BitmapFont::Glyph>, float> TextDrawer::get_glyphs_and_size(const std::string& text,
		BitmapFont* font, glm::vec2 scale)
{
	float x_off = 0;

	auto it = text.begin();

	std::vector<BitmapFont::Glyph> glyphs;

	while (it != text.end())
	{
		uint32_t code_point = utf8::unchecked::next(it);

		// TODO: Optimize this
		BitmapFont::Glyph gl;
		auto cit = font->chars.find(code_point);
		if (cit == font->chars.end())
		{
			// Question mark
			gl = font->chars[63];
		}
		else
		{
			gl = cit->second;
		}

		glyphs.push_back(gl);

		x_off += gl.xadvance * scale.x;

	}

	return std::make_pair(glyphs, x_off);
}

void TextDrawer::draw_text(const std::string& text, BitmapFont* font, glm::vec2 pos,
	glm::ivec2 screen, glm::vec4 color, glm::vec2 scale)
{
	draw_text_aligned(text, font, pos, Alignment::LEFT, screen, color, scale);
}

void TextDrawer::draw_text_aligned(const std::string& text, BitmapFont* font, glm::vec2 pos, Alignment alig,
	 glm::ivec2 screen, glm::vec4 color, glm::vec2 scale)
{

	auto[glyphs, x_off] =  get_glyphs_and_size(text, font, scale);

	glm::vec2 npos = pos;

	if (alig == CENTER)
	{
		npos.x -= x_off * 0.5f;
	}
	else if(alig == RIGHT)
	{
		npos.x -= x_off;
	}

	draw_glyphs(glyphs, font, npos, screen, color, scale);
}

int TextDrawer::get_size(const std::string & text, BitmapFont * font)
{
	int x_off = 0;

	auto it = text.begin();

	while (it != text.end())
	{
		uint32_t code_point = utf8::unchecked::next(it);

		// TODO: Optimize this
		BitmapFont::Glyph gl;
		auto cit = font->chars.find(code_point);
		if (cit == font->chars.end())
		{
			// Question mark
			gl = font->chars[63];
		}
		else
		{
			gl = cit->second;
		}

		x_off += gl.xadvance;

	}

	return x_off;
}

TextDrawer::TextDrawer()
{
	float vertices[] = {
		// Pos      // Tex
		0.0f, 1.0f, 0.0f, 1.0f,
		1.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 0.0f,

		0.0f, 1.0f, 0.0f, 1.0f,
		1.0f, 1.0f, 1.0f, 1.0f,
		1.0f, 0.0f, 1.0f, 0.0f
	};


	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);

	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	shader = osp->assets->get<Shader>("core", "shaders/font.vs");
}


TextDrawer::~TextDrawer()
{
}

void create_global_text_drawer()
{
	text_drawer = new TextDrawer();
}

void destroy_global_text_drawer()
{
	delete text_drawer;
}
