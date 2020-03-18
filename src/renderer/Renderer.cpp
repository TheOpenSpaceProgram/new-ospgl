#include "Renderer.h"
#include "../assets/AssetManager.h"


void Renderer::resize(int nwidth, int nheight, float nscale)
{
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


void Renderer::prepare_deferred()
{
	glDisable(GL_BLEND);

	if (wireframe)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

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

		glBindFramebuffer(GL_FRAMEBUFFER, gbuffer->g_buffer);

		glViewport(0, 0, swidth, sheight);
		doing_deferred = true;
		doing_forward = false;
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

}

void Renderer::prepare_forward(CameraUniforms& cu)
{
	glEnable(GL_BLEND);

	if (render_enabled)
	{

		glBindFramebuffer(GL_FRAMEBUFFER, fbuffer->fbuffer);
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
			l->do_pass(cu, gbuffer);
		}

		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDepthMask(GL_TRUE);
		glDepthFunc(GL_LESS);
		if (wireframe)
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}


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

		doing_deferred = false;
		doing_forward = false;

		glViewport(0, 0, width, height);

		glDepthFunc(GL_ALWAYS);
		texture_drawer->draw(fbuffer->tex_color_buffer,
			glm::vec2(0, 0),
			glm::vec2(width, height),
			glm::vec2(width, height), true);
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

void Renderer::finish()
{
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());



	glfwSwapBuffers(window);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

}

void Renderer::render()
{
	prepare_deferred();

	CameraUniforms c_uniforms = cam->get_camera_uniforms(swidth, sheight);

	if (render_enabled)
	{
		for (Drawable* d : deferred)
		{
			d->deferred_pass(c_uniforms);
		}

		prepare_forward(c_uniforms);

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
	}

	finish();
}


void Renderer::add_drawable(Drawable* d, std::string n_id)
{
	logger->check_important(!d->is_in_renderer(), "Tried to add an already added drawable");

	d->notify_add_to_renderer();

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

	all_drawables.push_back(d);
}


void Renderer::remove_drawable(Drawable* drawable)
{
	logger->check_important(drawable->is_in_renderer(), "Tried to remove a non-present drawable");

	drawable->notify_remove_from_renderer();

	for (auto it = all_drawables.begin(); it != all_drawables.end();)
	{
		if ((*it) == drawable)
		{
			it = all_drawables.erase(it);
		}
		else
		{
			it++;
		}
	}

	if (drawable->needs_deferred_pass())
	{
		for (auto it = deferred.begin(); it != deferred.end();)
		{
			if ((*it) == drawable)
			{
				it = deferred.erase(it);
			}
			else
			{
				it++;
			}
		}
	}

	if (drawable->needs_forward_pass())
	{
		for (auto it = forward.begin(); it != forward.end();)
		{
			if ((*it) == drawable)
			{
				it = forward.erase(it);
			}
			else
			{
				it++;
			}
		}
	}

	if (drawable->needs_gui_pass())
	{
		for (auto it = gui.begin(); it != gui.end();)
		{
			if ((*it) == drawable)
			{
				it = gui.erase(it);
			}
			else
			{
				it++;
			}
		}
	}

}

void Renderer::add_light(Light* light)
{
	logger->check_important(!light->is_added_to_renderer(), "Tried to add an already added light");
	light->set_added(true);
	
	lights.push_back(light);
}

void Renderer::remove_light(Light* light)
{
	logger->check_important(light->is_added_to_renderer(), "Tried to remove a non-added light");
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

int Renderer::get_width()
{
	return get_size().x;
}

int Renderer::get_height()
{
	return get_size().y;
}

glm::ivec2 Renderer::get_size()
{
	if (doing_forward || doing_deferred)
	{
		return glm::dvec2(swidth, sheight);
	}
	else
	{
		return glm::dvec2(width, height);
	}
}

Renderer::Renderer(cpptoml::table& settings)
{
	render_enabled = true;
	gbuffer = nullptr;

	std::string type = "windowed";

	width = settings.get_qualified_as<int>("renderer.width").value_or(512);
	height = settings.get_qualified_as<int>("renderer.height").value_or(512);
	scale = (float)settings.get_qualified_as<double>("renderer.scale").value_or(1.0);
	type = settings.get_qualified_as<std::string>("renderer.type").value_or("windowed");



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

	glfwMakeContextCurrent(window);
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		logger->fatal("Could not initialize glad");
	}

	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 130");
	ImGui::StyleColorsDark();
	// Font for the code editor
	ImFont* font_default = io.Fonts->AddFontDefault();
	ImFont* font_code = io.Fonts->AddFontFromFileTTF("./res/core/FiraCode-Regular.ttf", 16.0f);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glEnable(GL_DEPTH_CLAMP);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	resize(width, height, scale);

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
