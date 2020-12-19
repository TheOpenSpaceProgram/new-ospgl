#include "Renderer.h"
#include "../assets/AssetManager.h"

#define NANOVG_GL3_IMPLEMENTATION
#include <nanovg/nanovg_gl.h>
//?
#include "../universe/PlanetarySystem.h"

void Renderer::resize(int nwidth, int nheight, float nscale)
{
	if(nwidth <= 1 || nheight <= 1 || nscale <= 0.0f)
	{
		logger->warn("Invalid size for renderer, ignoring! ({}x{} (x{}))", nwidth, nheight, nscale);
		return;
	}

	if (gbuffer != nullptr)
	{
		delete gbuffer;
	}

	if (fbuffer != nullptr)
	{
		delete fbuffer;
	}
	
	

	width = nwidth;
	height = nheight;
	scale = nscale;
	swidth = (int)(width * scale);
	sheight = (int)(height * scale);

	gbuffer = new GBuffer((size_t)(width * scale), (size_t)(height * scale));
	fbuffer = new Framebuffer((size_t)(width * scale), (size_t)(height * scale), gbuffer->rbo);
}


void Renderer::deferred_bind(GLuint g_buffer, glm::ivec4 vport)
{
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glDisable(GL_BLEND);
	glBindFramebuffer(GL_FRAMEBUFFER, g_buffer);

	glViewport(vport.x, vport.y, vport.z, vport.w);
	doing_deferred = true;
	doing_forward = false;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::prepare_deferred()
{
	if (wireframe)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}


	int nwidth = 0, nheight = 0;

	glfwGetFramebufferSize(window, &nwidth, &nheight);

	if (nwidth <= 0 || nheight <= 0)
	{
		if (render_enabled)
		{
			logger->info("Window is out of focus, disabling rendering");
			render_enabled = false;
		}
	}
	else
	{
		if (!render_enabled)
		{
			logger->info("Window came into focus, enabling rendering");
			render_enabled = true;
		}
	}

	if (render_enabled)
	{
		if (nwidth != width || nheight != height)
		{
			resize(nwidth, nheight, scale);
		}

		deferred_bind(gbuffer->g_buffer, viewport);
	}

}

void Renderer::do_shadows(PlanetarySystem* system, glm::dvec3 camera_pos)
{
	// We disable depth clamping so that objects behind the lights are culled
	glDisable(GL_DEPTH_CLAMP);
	//glCullFace(GL_FRONT);
	for(Light* light : lights)
	{
		if(light->casts_shadows())
		{
			ShadowCamera shadow_cam = light->get_shadow_camera(camera_pos);

			glBindFramebuffer(GL_FRAMEBUFFER, shadow_cam.fbuffer);
			glViewport(0, 0, shadow_cam.size, shadow_cam.size);
			glClear(GL_DEPTH_BUFFER_BIT);


			for(Drawable* d : shadow)
			{
				d->shadow_pass(shadow_cam);
			}

			if(light->get_type() == Light::SUN)
			{
				if(system != nullptr)
				{
					// Far shadow (TODO)
				}	
			}

		}
	
	}
	glEnable(GL_DEPTH_CLAMP);
	//glCullFace(GL_BACK);
}

// TODO: Take GLuint g_buffer instead of GBuffer* g_buffer for consistency?
void Renderer::forward_bind(CameraUniforms& cu, GBuffer* g_buffer, GLuint f_buffer,
							glm::ivec4 vport, Light* only_light)
{
	glEnable(GL_BLEND);

	glBindFramebuffer(GL_FRAMEBUFFER, f_buffer);
	glViewport(vport.x, vport.y, vport.z, vport.w);
	glClear(GL_COLOR_BUFFER_BIT); //< Don't clear the depth buffer!

	doing_deferred = false;
	doing_forward = true;

	if (wireframe)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
	glDepthMask(GL_FALSE);
	glDepthFunc(GL_ALWAYS);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);

	// Do a pass for every light
	for (Light* l : lights)
	{
		glViewport(vport.x, vport.y, vport.z, vport.w);

		if(only_light)
		{
			l = only_light;
		}
		else if(l->needs_fullscreen_viewport())
		{
			glViewport(0, 0, swidth, sheight);
		}

		l->do_pass(cu, g_buffer);

		if(only_light)
		{
			break;
		}
	}

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LESS);
	if (wireframe)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}

	glViewport(vport.x, vport.y, vport.z, vport.w);
}

void Renderer::prepare_forward(CameraUniforms& cu)
{
	if (render_enabled)
	{
		forward_bind(cu, gbuffer, fbuffer->fbuffer, viewport, nullptr);

	}
}

void Renderer::prepare_gui()
{
	if (wireframe)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}


	if (render_enabled)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// Clear the stencil buffer for nanovg
		
		glClear(GL_STENCIL_BUFFER_BIT);

		doing_deferred = false;
		doing_forward = false;

		glViewport(0, 0, width, height);

		glDepthFunc(GL_ALWAYS);
		texture_drawer->draw(fbuffer->tex_color_buffer,
			glm::vec2(0, 0),
			glm::vec2(width, height),
			glm::vec2(width, height), hdr.data, true);
		glDepthFunc(GL_LESS);
	}
}

void Renderer::do_debug(CameraUniforms &cu)
{
	glm::dmat4 proj_view = cu.proj_view;
	glm::dmat4 c_model = cu.c_model;
	float far_plane = cu.far_plane;

	// Don't forget to draw the debug shapes!
	debug_drawer->render(proj_view, c_model, far_plane);

}

void Renderer::do_imgui()
{
	ImGui::Render();

	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Renderer::finish()
{
	glfwSwapBuffers(window);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

}

void Renderer::render_env_face(glm::dvec3 sample_pos, size_t face)
{
	CameraUniforms c_uniforms;
	float far_plane = 1e16f;

	glm::dmat4 views[] =
	{
		glm::lookAt(glm::dvec3(0.0f), glm::dvec3(1.0f,  0.0f,  0.0f), glm::dvec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::dvec3(0.0f), glm::dvec3(-1.0f,  0.0f,  0.0f), glm::dvec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::dvec3(0.0f), glm::dvec3(0.0f,  1.0f,  0.0f), glm::dvec3(0.0f,  0.0f,  1.0f)),
		glm::lookAt(glm::dvec3(0.0f), glm::dvec3(0.0f, -1.0f,  0.0f), glm::dvec3(0.0f,  0.0f, -1.0f)),
		glm::lookAt(glm::dvec3(0.0f), glm::dvec3(0.0f,  0.0f,  1.0f), glm::dvec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::dvec3(0.0f), glm::dvec3(0.0f,  0.0f, -1.0f), glm::dvec3(0.0f, -1.0f,  0.0f))
	};

	glm::dmat4 proj = glm::perspective(glm::radians(90.0), 1.0, 0.01, (double)far_plane);
	glm::dmat4 view = views[face];
	glm::dmat4 proj_view = proj * view;
	c_uniforms.proj = proj;
	c_uniforms.view = view;
	c_uniforms.proj_view = proj_view;
	c_uniforms.c_model = glm::translate(glm::dmat4(1.0), -sample_pos);
	c_uniforms.tform = proj * view * c_uniforms.c_model;
	c_uniforms.far_plane = far_plane;
	c_uniforms.cam_pos = sample_pos;

	c_uniforms.screen_size = glm::vec2(ibl_source->resolution, ibl_source->resolution);
	c_uniforms.iscreen_size = glm::ivec2(c_uniforms.screen_size);


	glm::dvec4 vport = glm::dvec4(0, 0, ibl_source->resolution, ibl_source->resolution);
	deferred_bind(env_gbuffer->g_buffer, vport);

	for (Drawable* d : env_map)
	{
		d->deferred_pass(c_uniforms, true);
	}

	// TODO: Disable texture creation in the framebuffer to avoid unused space
	glBindFramebuffer(GL_FRAMEBUFFER, env_fbuffer->fbuffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, ibl_source->id, 0);

	forward_bind(c_uniforms, env_gbuffer, env_fbuffer->fbuffer, vport, lights[0]);

	// Sort forward drawables
	std::sort(env_map.begin(), env_map.end(), [](Drawable* a, Drawable* b)
	{
		// This puts the higher priority drawables FIRST in the array
		return a->get_forward_priority() > b->get_forward_priority();
	});

	for (Drawable* d : env_map)
	{
		d->forward_pass(c_uniforms, true);
	}

	// TODO: Find a way to generate the mipmap only for one face
	// Although the perfomance hit is pretty much negligible on
	// my laptop's GPUs (GTX 1650 and Intel HD 620)
	glBindTexture(GL_TEXTURE_CUBE_MAP, ibl_source->id);
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

	ibl_source->generate_ibl_irradiance(64, 128, face);

}


void Renderer::render(PlanetarySystem* system)
{
	if(!cam)
	{
		logger->warn("No camera attached to renderer"); 
		return;
	}

	// Calculate viewport and widths
	if(override_viewport.x >= 0.0)
	{
		double xdiff = override_viewport.z - override_viewport.x;
		double ydiff = override_viewport.w - override_viewport.y;
		
		rswidth = (int)(swidth * xdiff);
		rsheight = (int)(sheight * ydiff);
	
		int x0 = (int)(swidth * override_viewport.x);
		int y0 = (int)(sheight * override_viewport.y);

		viewport = glm::ivec4(x0, y0, rswidth, rsheight);	
	}
	else
	{
		// Fullscreen
		rswidth = swidth;
		rsheight = sheight;
		viewport = glm::ivec4(0, 0, swidth, sheight);
	}

	CameraUniforms c_uniforms = cam->get_camera_uniforms(rswidth, rsheight);
	if(ibl_source && ibl_source->irradiance && ibl_source->specular)
	{
		c_uniforms.irradiance = ibl_source->irradiance->id;
		c_uniforms.specular = ibl_source->specular->id;
	}
	c_uniforms.brdf = brdf->id;

	
	prepare_deferred();

	if (render_enabled)
	{
		for (Drawable* d : deferred)
		{
			d->deferred_pass(c_uniforms);
		}

		do_shadows(system, c_uniforms.cam_pos);
		prepare_forward(c_uniforms);

		// Sort forward drawables
		std::sort(forward.begin(), forward.end(), [](Drawable* a, Drawable* b)
		{
			// This puts the higher priority drawables FIRST in the array
			return a->get_forward_priority() > b->get_forward_priority();
		});

		for (Drawable* d : forward)
		{
			d->forward_pass(c_uniforms);
		}

		if (debug_drawer->debug_enabled)
		{
			do_debug(c_uniforms);
		}

		prepare_gui();

		for (Drawable* d : gui)
		{
			d->gui_pass(c_uniforms);
		}

		// Draw nanoVG 
		nvgEndFrame(vg);
		nvgEndFrameExt(vg);

		// Sanitize OpenGL after nanoVG call
		glEnable(GL_DEPTH_TEST);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glEnable(GL_CULL_FACE);
		glBindTexture(GL_TEXTURE_2D, 0);

		// Draw text_drawer added text
	}

}


void Renderer::add_drawable(Drawable* d, std::string n_id)
{
	logger->check(!d->is_in_renderer(), "Tried to add an already added drawable");

	drawable_uid++,
	d->notify_add_to_renderer(drawable_uid);

	if (n_id != "")
	{
		d->set_drawable_id(n_id);
	}

	if (d->needs_deferred_pass())
	{
		deferred.push_back(d);
	}

	if (d->needs_forward_pass())
	{
		forward.push_back(d);
	}

	if (d->needs_gui_pass())
	{
		gui.push_back(d);
	}

	if(d->needs_shadow_pass())
	{
		shadow.push_back(d);
	}

	if(d->needs_far_shadow_pass())
	{
		far_shadow.push_back(d);
	}

	if(d->needs_env_map_pass())
	{
		env_map.push_back(d);
	}

	all_drawables.push_back(d);
}
static void remove_from(std::vector<Drawable*>& array, Drawable* d)
{
	for (auto it = array.begin(); it != array.end();)
	{
		if ((*it) == d)
		{
			it = array.erase(it);
		}
		else
		{
			it++;
		}
	}
}

void Renderer::remove_drawable(Drawable* drawable)
{
	logger->check(drawable->is_in_renderer(), "Tried to remove a non-present drawable");

	drawable->notify_remove_from_renderer();

	remove_from(all_drawables, drawable);

	if (drawable->needs_deferred_pass())
	{
		remove_from(deferred,  drawable);
	}

	if (drawable->needs_forward_pass())
	{
		remove_from(forward, drawable);
	}

	if (drawable->needs_gui_pass())
	{
		remove_from(gui, drawable);
	}

	if (drawable->needs_shadow_pass())
	{
		remove_from(shadow, drawable);
	}

	if(drawable->needs_far_shadow_pass())
	{
		remove_from(far_shadow, drawable);
	}

	if(drawable->needs_env_map_pass())
	{
		remove_from(env_map, drawable);
	}
}

void Renderer::add_light(Light* light)
{
	logger->check(!light->is_added_to_renderer(), "Tried to add an already added light");
	light->set_added(true);
	
	lights.push_back(light);
}

void Renderer::remove_light(Light* light)
{
	logger->check(light->is_added_to_renderer(), "Tried to remove a non-added light");
	light->set_added(false);

	for (auto it = lights.begin(); it != lights.end(); )
	{
		if (*it == light)
		{
			it = lights.erase(it);
		}
		else
		{
			it++;
		}
	}
}

int Renderer::get_width(bool gui)
{
	return get_size(gui).x;
}

int Renderer::get_height(bool gui)
{
	return get_size(gui).y;
}

glm::ivec2 Renderer::get_size(bool gui)
{
	if(gui)
	{
		return glm::dvec2(width, height);
	}

	if (doing_forward || doing_deferred)
	{
		return glm::dvec2(rswidth, rsheight);
	}
	else
	{
		return glm::dvec2(width, height);
	}
}

static void open_gl_debug_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* msg, const void* user_param)
{
	std::string str_msg;
	for(size_t i = 0; i < length; i++)
	{
		str_msg.push_back(msg[i]);
	}

	if(severity == GL_DEBUG_SEVERITY_LOW)
	{
		logger->info("{}", str_msg);
	}
	else if(severity == GL_DEBUG_SEVERITY_MEDIUM)
	{
		logger->warn("{}", str_msg);
	}
	else if(severity == GL_DEBUG_SEVERITY_HIGH)
	{
		logger->fatal("{}", str_msg);
	}
	else
	{
		logger->debug("{}", str_msg);
	}
}

Renderer::Renderer(cpptoml::table& settings)
{
	drawable_uid = 0;
	cam = nullptr;

	override_viewport = glm::dvec4(-1.0, -1.0, -1.0, -1.0);

	render_enabled = true;
	gbuffer = nullptr;
	fbuffer = nullptr;
	wireframe = false;

	doing_forward = false;
	doing_deferred = false;

	std::string type = "windowed";

	width = settings.get_qualified_as<int>("renderer.width").value_or(512);
	height = settings.get_qualified_as<int>("renderer.height").value_or(512);
	scale = (float)settings.get_qualified_as<double>("renderer.scale").value_or(1.0);
	type = settings.get_qualified_as<std::string>("renderer.type").value_or("windowed");



	window = nullptr;

	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	if (type == "windowed")
	{
		window = glfwCreateWindow(width, height, "New OSP", NULL, NULL);
	}
	// TODO: Allow any monitor from settings
	else if (type == "fullscreen")
	{
		window = glfwCreateWindow(width, height, "New OSP", glfwGetPrimaryMonitor(), NULL);
	}
	else if (type == "windowed fullscreen")
	{
		GLFWmonitor* monitor = glfwGetPrimaryMonitor();

		const GLFWvidmode* mode = glfwGetVideoMode(monitor);
		glfwWindowHint(GLFW_RED_BITS, mode->redBits);
		glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
		glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
		glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
		window = glfwCreateWindow(mode->width, mode->height, "New OSP", monitor, NULL);
	}

	if(window == nullptr)
	{
		const char* description;
		int code = glfwGetError(&description);
		
		logger->error("Error initializing GLFW window: {}", description);
		logger->fatal("Failed to initialize GLFW");
	}

	glfwMakeContextCurrent(window);
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		logger->fatal("Could not initialize glad");
	}

	// Print context info
	logger->info("OpenGL Version: {}", glGetString(GL_VERSION));
	logger->info("GLSL Version: {}", glGetString(GL_SHADING_LANGUAGE_VERSION));
	logger->info("OpenGL Vendor: {}",  glGetString(GL_VENDOR));
	logger->info("OpenGL Renderer: {}", glGetString(GL_RENDERER));
	

	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 130");
	ImGui::StyleColorsDark();
	
	ImFont* font_default = io.Fonts->AddFontDefault();

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glEnable(GL_DEPTH_CLAMP);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
#ifdef ENABLE_GL_DEBUG
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(open_gl_debug_callback, (void*)0);
#endif
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Create the nanoVG context
	vg = nvgCreateGL3(NVG_ANTIALIAS);
	nvgCreateExt(vg);
	//vg = nvgCreateGL3(0);

	// Load the default NVG fonts (TODO: Use generic names for this or move it over to package.lua)
	nvgCreateFont(vg, "regular", (osp->assets->res_path + "core/fonts/Roboto-Regular.ttf").c_str());
	nvgCreateFont(vg, "bold", (osp->assets->res_path + "core/fonts/Roboto-Bold.ttf").c_str());
	nvgCreateFont(vg, "light", (osp->assets->res_path + "core/fonts/Roboto-Light.ttf").c_str());
	nvgCreateFont(vg, "medium", (osp->assets->res_path + "core/fonts/Roboto-Medium.ttf").c_str());
	// This font is meant to be used at exactly size 12
	nvgCreateFont(vg, "tiny", (osp->assets->res_path + "core/fonts/ProggyTinySZ.ttf").c_str());

	resize(width, height, scale);

	auto quality_toml = settings.get_table_qualified("renderer.quality");
	SerializeUtil::read_to(*quality_toml, quality);

	hdr = AssetHandle<Shader>("core:shaders/hdr.vs");
	brdf = AssetHandle<Image>("core:shaders/ibl/brdf.png");

}

Renderer::~Renderer()
{
	if (gbuffer != nullptr)
	{
		delete gbuffer;
	}

	glfwDestroyWindow(window);
	glfwTerminate();
}

void Renderer::set_ibl_source(Cubemap* cubemap)
{
	delete env_fbuffer;
	delete env_gbuffer;

	if(cubemap == nullptr)
	{
		cubemap = new Cubemap(quality.env_map_size, true);

		env_gbuffer = new GBuffer(cubemap->resolution, cubemap->resolution);
		env_fbuffer = new Framebuffer(cubemap->resolution, cubemap->resolution, env_gbuffer->rbo);
	}

	this->ibl_source = cubemap;
}


