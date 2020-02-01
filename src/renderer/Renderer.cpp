#include "Renderer.h"



void Renderer::resize(int nwidth, int nheight, float nscale)
{
	if (fbuffer != nullptr)
	{
		delete fbuffer;
	}

	width = nwidth;
	height = nheight;
	scale = nscale;
	swidth = (int)(width * scale);
	sheight = (int)(height * scale);

	fbuffer = new Framebuffer((size_t)(width * scale), (size_t)(height * scale));
}

void Renderer::prepare_draw()
{
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

		fbuffer->bind();
		doing_fbuffer = true;
		glViewport(0, 0, swidth, sheight);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

}

void Renderer::prepare_gui()
{
	if (render_enabled)
	{
		fbuffer->unbind();
		doing_fbuffer = false;

		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// TODO: Add support for post-processing
		texture_drawer->draw(fbuffer->tex_color_buffer,
			glm::vec2(0, 0),
			glm::vec2(width, height),
			glm::vec2(width, height), true);
	}
}

void Renderer::finish()
{

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());



	glfwSwapBuffers(window);

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
	if (doing_fbuffer)
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
	fbuffer = nullptr;

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
	if (fbuffer != nullptr)
	{
		delete fbuffer;
	}

	glfwDestroyWindow(window);
	glfwTerminate();
}
