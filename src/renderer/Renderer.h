#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

#include "util/Framebuffer.h"
#include "util/TextureDrawer.h"
#include "util/DebugDrawer.h"
#include "util/GBuffer.h"

#include "camera/Camera.h"
#include "Drawable.h"
#include "LuaDrawable.h"
#include "lighting/ShadowCamera.h"
#include "lighting/Light.h"
#include <nanovg/nanovg.h>

#include <assets/Cubemap.h>

#include "RendererQuality.h"


//#define ENABLE_GL_DEBUG

class PlanetarySystem;
class Entity;
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
// 		Note: Sunlight is approximated as a directional light,
// 			eclipses could be slightly different	
class Renderer
{
private:
	AssetHandle<Shader> hdr;
	AssetHandle<Image> brdf;

	Framebuffer* fbuffer;
	GBuffer* gbuffer;

	Framebuffer* env_fbuffer = nullptr;
	GBuffer* env_gbuffer = nullptr;

	// Real size of the window
	int width, height;
	// Size of the scaled rendering (may be less than 100%)
	int swidth, sheight;
	// Size of the current scaled viewport, including override_viewport
	int rswidth, rsheight;
	float scale;

	bool doing_forward;
	bool doing_deferred;

	// Everytime a drawable is added the number increases, even if the
	// drawable was added before
	GLint drawable_uid;

	glm::ivec4 viewport;

	// Setups OpenGL to draw to the gbuffer
	void prepare_deferred();
	void deferred_bind(GLuint g_buffer, glm::ivec4 viewport);

	void do_shadows(PlanetarySystem* system, glm::dvec3 cam_pos);

	// Setups OpenGL to draw to the normal fullscreen fbuffer
	// and draws the gbuffer to the screen (also updating depth buffer
	// for the forward rendering stuff)
	void prepare_forward(CameraUniforms& cu);
	void forward_bind(CameraUniforms& cu, GBuffer* g_buffer, GLuint f_buffer, glm::ivec4 viewport, bool is_env_pass);

	// Draws the framebuffer to screen and prepares OpenGL
	// to draw directly to backbuffer
	void prepare_gui();

	void do_debug(CameraUniforms& cu);

	// Used for lua lifetime management, we don't manage memory here
	// but as all drawables are created from lua they are eventually garbage
	// collected once the shared_ptr goes null
	std::vector<Drawable*> deferred;
	std::vector<Drawable*> forward;
	std::vector<Drawable*> gui;
	std::vector<Drawable*> shadow;
	std::vector<Drawable*> far_shadow;
	std::vector<Drawable*> env_map;
	std::vector<std::shared_ptr<Drawable>> all_drawables;

	std::vector<std::shared_ptr<Light>> lights;

	// Used for env_map sampling
	glm::dvec3 env_last_pos;
	double env_last_time;
	size_t env_frames;
	size_t env_face;
	bool env_first;
	bool env_enabled;

public:
	glm::dvec3 env_sample_pos;

	// Only if using a cubemap as ibl source, to prevent unloading
	AssetHandle<Cubemap> ibl_source_asset;
	Cubemap* ibl_source = nullptr;

	// If cubemap is a null asset, ibl is disabled. Takes ownership
	void set_ibl_source(AssetHandle<Cubemap>&& cubemap);

	// Enables dynamic env_sampling
	void enable_env_sampling();

	RendererQuality quality;

	// If it's not (0,0,1,1), it will apply a glViewport
	// to forward and deferred (GUI is always full) adjusted
	// for these coeficitents
	// You should specify a rectangle, (x,y) being min and (z,w) max
	glm::dvec4 override_viewport;
	
	Camera* cam;

	bool wireframe;

	bool render_enabled;

	GLFWwindow* window;

	NVGcontext* vg;

	void resize(int nwidth, int nheight, float scale);

	// We need the PlanetarySystem for advanced shadowing, pass
	// nullptr if you are not currently on a scene which requires 
	// complex shadows (landscape shadows)
	void render(PlanetarySystem* system);

	void render_env_face(glm::dvec3 sample_pos, size_t face);

	void env_map_sample();

	// Must always be called (except on headless), even if nothing
	// is rendering to the screen
	void do_imgui();

	void finish();


	void add_drawable(std::shared_ptr<Drawable> d, std::string n_id);
	void remove_drawable(Drawable* d);
	void remove_all_drawables();

	// Lights are different
	void add_light(std::shared_ptr<Light> light);
	void remove_light(Light* light);
	void remove_all_lights();

	// This is a workaround for an issue in sol where shared_ptr cannot be cast to one another
	template<typename T>
	void add_drawable_lua(std::shared_ptr<T> d){ add_drawable(d, ""); }

	void add_drawable_entity_lua(Entity* ent);

	template<typename T>
	void add_light_lua(std::shared_ptr<T> d){ add_light(d); }

	// removes all lights and drawables
	void clear()
	{
		remove_all_drawables();
		remove_all_lights();
	}

	int get_width(bool gui = false);
	int get_height(bool gui = false);

	glm::ivec2 get_size(bool gui = false);

	GLuint get_brdf_id() const { return brdf->id; }

	// Initializes OpenGL. Don't forget to set the camera afterwards
	Renderer(cpptoml::table& settings);
	~Renderer();
};
