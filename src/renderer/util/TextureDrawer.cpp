#include "TextureDrawer.h"
#include "../../assets/AssetManager.h"

TextureDrawer* texture_drawer;

void create_global_texture_drawer()
{
	texture_drawer = new TextureDrawer();
}

void destroy_global_texture_drawer()
{
	delete texture_drawer;
}

void TextureDrawer::draw(GLuint tex, glm::vec2 pos, glm::vec2 size, glm::ivec2 screen_size, bool vflip)
{
	draw(tex, pos, size, screen_size, this->shader, true);
}

void TextureDrawer::draw(GLuint tex, glm::vec2 pos, glm::vec2 size, glm::ivec2 screen_size, Shader* cshader, bool vflip)
{
	glm::mat4 view = glm::ortho(0.0f, (float)screen_size.x, (float)screen_size.y, 0.0f, -1.0f, 1.0f);
	glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(pos, 0.0f));
	//model = glm::translate(model, glm::vec3(-size.x, -size.y, 0.0f));
	model = glm::scale(model, glm::vec3(size, 1.0f));
	//model = glm::translate(model, glm::vec3(0.5f, 0.5f, 0.0f));

	glm::mat4 tform = view * model;

	cshader->use();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex);

	cshader->setInt("tex", 0);
	cshader->setMat4("tform", tform);
	cshader->setInt("vflip", vflip);

	glBindVertexArray(vao);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void TextureDrawer::issue_fullscreen_rectangle()
{
	glBindVertexArray(full_vao);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}

void TextureDrawer::issue_rectangle(glm::vec2 pos0, glm::vec2 size)
{

}


TextureDrawer::TextureDrawer()
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

	float full_vertices[] = {
		// Pos              // Tex
		-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
		-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
		 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
		 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
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

	glGenVertexArrays(1, &full_vao);
	glGenBuffers(1, &full_vbo);
	glBindVertexArray(full_vao);
	glBindBuffer(GL_ARRAY_BUFFER, full_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(full_vertices), full_vertices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	shader = assets->get<Shader>("core", "shaders/simple_sprite.vs");
}

TextureDrawer::~TextureDrawer()
{

}
