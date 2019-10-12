#include <sol.hpp>
#include <iostream>
#include "util/Logger.h"
#include "util/Timer.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "util/InputUtil.h"
#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

#include "tools/planet_editor/PlanetEditor.h"


#define WIDTH 1500
#define HEIGHT 900

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
	createGlobalLogger();

	logger->info("Starting OSP");

	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "New OSP", NULL, NULL);
	glfwMakeContextCurrent(window);
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		logger->fatal("Could not initialize glad");
	}

	createGlobalAssetManager();


	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 130");
	ImGui::StyleColorsDark();

	// Font for the code editor
	ImFont* font_default = io.Fonts->AddFontDefault();
	ImFont* font_code = io.Fonts->AddFontFromFileTTF("./res/FiraCode-Regular.ttf", 16.0f);

	Timer dtt = Timer();
	float dt = 0.0f;
	float t = 0.0f;

	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);

	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	PlanetEditor editor = PlanetEditor(window, "test");

	input = new InputUtil();
	input->setup(window);

	while (!glfwWindowShouldClose(window))
	{
		input->update(window);

		glfwPollEvents();


		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();


		editor.update(dt, font_code);



		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);


		editor.render(WIDTH, HEIGHT);
		
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	
		glfwSwapBuffers(window);


		dt = (float)dtt.restart();
		t += dt;
	}

	logger->info("Ending OSP");

	delete input;

	glfwDestroyWindow(window);
	glfwTerminate();

	destroyGlobalAssetManager();
	destroyGlobalLogger();

}