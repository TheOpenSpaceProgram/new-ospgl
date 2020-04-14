#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

#include <util/render/Framebuffer.h>
#include <util/render/TextureDrawer.h>
#include <util/DebugDrawer.h>
#include "GBuffer.h"

#include "camera/Camera.h"
#include "Drawable.h"
#include "lighting/ShadowCamera.h"
#include "lighting/Light.h"
#include <nanovg/nanovg.h>

//#define ENABLE_GL_DEBUG

class PlanetarySystem;


// Allows scaled rendering of the 3D scene to a framebuffer
// and then UI drawing (in real resolution) over it
// It first does a deferred rendering pass, and then a
// forward rendering pass for transparent objects 
// (aka atmospheres), and for GUIs
// 
// NOTE: It does not support lights for forward objects
// TODO: Maybe support them? I don't think they are neccesary
// 
// Shadows:
// 		SunLights use a special system (they need two cascades, one 
// 		for landscape shadows (far) and another for vehicle shadows (close))
// 
// 		TODO: Other lights
// 	
// 		Note: Sunlight is approximated as a directional light,
// 			eclipses could be slightly different	


class Renderer
{
private:
	Framebuffer* fbuffer;
	GBuffer* gbuffer;

	// Real size of the window
	int width, height;
	// Size of the scaled rendering (may be less than 100%)
	int swidth, sheight;
	// Size of the current scaled viewport, including override_viewport
	int rswidth, rsheight;
	float scale;

	bool doing_forward;
	bool doing_deferred;

	glm::ivec4 viewport;

	// Setups OpenGL to draw to the gbuffer
	void prepare_deferred();

	void do_shadows(PlanetarySystem* system, glm::dvec3 cam_pos);

	// Setups OpenGL to draw to the normal fullscreen fbuffer
	// and draws the gbuffer to the screen (also updating depth buffer
	// for the forward rendering stuff)
	void prepare_forward(CameraUniforms& cu);

	// Draws the framebuffer to screen and prepares OpenGL
	// to draw directly to backbuffer
	void prepare_gui();

	void do_debug(CameraUniforms& cu);

	std::vector<Drawable*> deferred;
	std::vector<Drawable*> forward;
	std::vector<Drawable*> gui;
	std::vector<Drawable*> shadow;
	std::vector<Drawable*> far_shadow;
	std::vector<Drawable*> all_drawables;

	std::vector<Light*> lights;

public:

	// If values are positive, it will apply a glViewport
	// to forward and deferred (GUI is always full) adjusted
	// for these coeficitents
	// You should specify a rectangle, (x,y) being min and (z,w) max
	glm::dvec4 override_viewport;
	
	// NanoVG is used to draw GUIs in a similar way to ImGui
	// but for final GUIs (not debug interfaces)
	NVGcontext* vg;

	Camera* cam;

	bool wireframe;

	bool render_enabled;

	GLFWwindow* window;

	void resize(int nwidth, int nheight, float scale);

	// We need the PlanetarySystem for advanced shadowing, pass
	// nullptr if you are not currently on a scene which requires 
	// complex shadows (landscape shadows)
	void render(PlanetarySystem* system);

	// Must always be called (except on headless), even if nothing
	// is rendering to the screen
	void do_imgui();

	void finish();


	void add_drawable(Drawable* d, std::string n_id = "");
	void remove_drawable(Drawable* d);

	// Lights are different
	void add_light(Light* light);
	void remove_light(Light* light);

	int get_width(bool gui = false);
	int get_height(bool gui = false);

	glm::ivec2 get_size(bool gui = false);


	// Initializes OpenGL. Don't forget to set the camera afterwards
	Renderer(cpptoml::table& settings);
	~Renderer();
};
