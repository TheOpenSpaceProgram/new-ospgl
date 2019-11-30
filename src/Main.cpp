#include <sol.hpp>
#include <iostream>
#include "util/Logger.h"
#include "util/Timer.h"
#include "util/DebugDrawer.h"
#include "util/render/TextureDrawer.h"

#include "game/ui/Navball.h"
#include "assets/Config.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "util/InputUtil.h"
#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

#include "tools/planet_editor/PlanetEditor.h"
#include "universe/PlanetarySystem.h"
#include "universe/Date.h"

InputUtil* input;

/*
void character_callback(GLFWwindow* window, unsigned int codepoint)
{
	ImGuiIO& io = ImGui::GetIO();
	//io.InputQueueCharacters.push_back(codepoint);
	io.AddInputCharacter(codepoint);
}
*/

int main(void)
{

	int width = 1366;
	int height = 768;

	create_global_logger();

	logger->info("Starting OSP");

	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	GLFWwindow* window = glfwCreateWindow(width, height, "New OSP", NULL, NULL);
	glfwMakeContextCurrent(window);
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		logger->fatal("Could not initialize glad");
	}

	create_global_asset_manager();
	create_global_debug_drawer();
	create_global_texture_drawer();

	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 130");
	ImGui::StyleColorsDark();

	// Font for the code editor
	ImFont* font_default = io.Fonts->AddFontDefault();
	ImFont* font_code = io.Fonts->AddFontFromFileTTF("./res/core/FiraCode-Regular.ttf", 16.0f);

	Timer dtt = Timer();
	double dt = 0.0;

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);

	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	//PlanetEditor editor = PlanetEditor(window, "earth");

	input = new InputUtil();
	input->setup(window);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	PlanetarySystem system;
	assets->get_from_path<Config>("rss:systems/test_system.toml")->read_to(system);

	system.compute_sois(0.0);
	debug_drawer->debug_enabled = true;

	//Date start_date = Date(2000, Date::MAY, 31);
	Date start_date = Date(2019, Date::SEPTEMBER, 21);

	start_date.day_decimal = (19.0 + 27.0 / 60.0) / 24.0;

	system.t = start_date.to_seconds();
	system.t = 0.0;
	logger->info("Starting at: {}", start_date.to_string());

	system.init();

	Navball navball;


	SystemPointer center_ptr = SystemPointer(&system, 0, true);

	SystemPointer secondary_ptr = SystemPointer(&system, "Earth");
	secondary_ptr.id = 3;
	secondary_ptr.is_vessel = false;

	ReferenceFrame ref(center_ptr);
	ref.mode = ReferenceFrame::INERTIAL;
	ref.center2 = secondary_ptr;


	system.camera.frame = &ref;

	while (!glfwWindowShouldClose(window))
	{
		input->update(window);

		glfwGetWindowSize(window, &width, &height);
		glViewport(0, 0, width, height);

		glfwPollEvents();


		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();



		system.update(dt);
		//editor.update((float)dt, font_code);
		ImGui::Begin("Date");

		ImGui::Text("%s", Date(system.t).to_string().c_str());
		ImGui::InputDouble("Timewarp", &system.timewarp);

		ImGui::End();

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);


		
		system.render(width, height);
		//editor.render(width, height);

		navball.draw_to_texture(system.vessels[0], ref);
		navball.draw_to_screen({ width, height });

		system.render_debug(width, height);

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	
		glfwSwapBuffers(window);

		dt = dtt.restart();


	}

	logger->info("Ending OSP");

	delete input;

	glfwDestroyWindow(window);
	glfwTerminate();

	destroy_global_texture_drawer();
	destroy_global_asset_manager();
	destroy_global_logger();


}