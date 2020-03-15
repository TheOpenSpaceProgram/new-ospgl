#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

#include "../util/render/Framebuffer.h"
#include "../util/render/TextureDrawer.h"
#include "../util/DebugDrawer.h"
#include "GBuffer.h"

#include "camera/Camera.h"
#include "Drawable.h"
#include "lighting/Light.h"

// Allows scaled rendering of the 3D scene to a framebuffer
// and then UI drawing (in real resolution) over it
// It first does a deferred rendering pass, and then a
// forward rendering pass for transparent objects 
// (aka atmospheres), and for GUIs
// 
// NOTE: It does not support lights for forward objects
// TODO: Maybe support them? I don't think they are neccesary
class Renderer
{
private:
	Framebuffer* fbuffer;
	GBuffer* gbuffer;

	int width, height;
	int swidth, sheight;
	float scale;

	bool doing_forward;
	bool doing_deferred;


	// Setups OpenGL to draw to the gbuffer
	void prepare_deferred();

	// Setups OpenGL to draw to the normal fullscreen fbuffer
	// and draws the gbuffer to the screen (also updating depth buffer
	// for the forward rendering stuff)
	void prepare_forward(glm::dvec3 cam_pos);

	// Draws the framebuffer to screen and prepares OpenGL
	// to draw directly to backbuffer
	void prepare_gui();

	void do_debug(CameraUniforms& cu);

	std::vector<Drawable*> deferred;
	std::vector<Drawable*> forward;
	std::vector<Drawable*> gui;
	std::vector<Drawable*> all_drawables;

	std::vector<Light*> lights;

public:


	Camera* cam;

	bool wireframe;

	bool render_enabled;

	GLFWwindow* window;

	void resize(int nwidth, int nheight, float scale);

	void render();

	void finish();


	void add_drawable(Drawable* d, std::string n_id = "");
	void remove_drawable(Drawable* d);

	// Lights are different
	void add_light(Light* light);
	void remove_light(Light* light);

	int get_width();
	int get_height();

	glm::ivec2 get_size();

	// Initializes OpenGL. Don't forget to set the camera afterwards
	Renderer(cpptoml::table& settings);
	~Renderer();
};
