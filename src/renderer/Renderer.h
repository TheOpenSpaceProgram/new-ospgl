#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

#include "../util/render/Framebuffer.h"
#include "../util/render/TextureDrawer.h"


// Allows scaled rendering of the 3D scene to a framebuffer
// and then UI drawing (in real resolution) over it
class Renderer
{
private:
	Framebuffer* fbuffer;

	int width, height;
	int swidth, sheight;
	float scale;

	bool doing_fbuffer;




public:

	bool wireframe;

	bool render_enabled;

	GLFWwindow* window;


	void resize(int nwidth, int nheight, float scale);


	// Setups OpenGL to draw to the fullscreen framebuffer
	void prepare_draw();

	// Draws the framebuffer to screen and prepares OpenGL
	// to draw directly to backbuffer
	void prepare_gui();

	void finish();

	int get_width();
	int get_height();

	glm::ivec2 get_size();

	// Initializes OpenGL
	Renderer(cpptoml::table& settings);
	~Renderer();
};

