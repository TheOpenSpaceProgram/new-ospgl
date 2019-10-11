#include <sol.hpp>
#include <iostream>
#include "util/Logger.h"
#include "util/Timer.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

#include "assets/AssetManager.h"
#include "planet_mesher/quadtree/QuadTreePlanet.h"
#include "planet_mesher/mesher/PlanetTileServer.h"
#include "planet_mesher/renderer/PlanetRenderer.h"

#define WIDTH 1500
#define HEIGHT 900

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
	ImGui::StyleColorsDark();

	float x = 0.2f;

	size_t depth = 0;

	std::string script = assets->loadString("res/test.lua");

	QuadTreePlanet planet;
	planet.set_wanted_subdivide(glm::dvec2(x, 0.5), PX, depth);
	PlanetTileServer server(script);

	PlanetRenderer renderer;

	Timer dtt = Timer();
	float dt = 0.0f;
	float t = 0.0f;

	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);

	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	while (!glfwWindowShouldClose(window))
	{
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		server.update(planet);
		planet.dirty = false;

		planet.update(server);

		if (ImGui::Begin("Planet"))
		{

			if (ImGui::CollapsingHeader("Quadtree", ImGuiTreeNodeFlags_DefaultOpen))
			{
				planet.do_imgui(server);
			}

			if (ImGui::CollapsingHeader("Tile Server"))
			{
				server.do_imgui();
			}

			bool move = false;
			if (ImGui::Button("Move"))
			{
				x += 0.2f;
				move = true;
			}

			ImGui::SameLine();
			if (ImGui::Button("Back"))
			{
				x -= 0.2f;
				move = true;
			}

			ImGui::SameLine();
			if (ImGui::Button("Go Deep"))
			{
				depth++;
				move = true;
			}

			ImGui::SameLine();
			if (ImGui::Button("Go Shallow"))
			{
				if (depth >= 1)
				{
					depth--;
					move = true;
				}
			}

			if (move)
			{
				planet.set_wanted_subdivide(glm::dvec2(x, 0.5), PX, depth);
			}


		}
		ImGui::End();

		glm::mat4 proj = glm::perspective(glm::radians(80.0f), (float)WIDTH / (float)HEIGHT, 0.01f, 800.0f);
		glm::mat4 view = glm::lookAt(glm::vec3(sin(t * 0.4f) * 2.5f, 0.0f, cos(t * 0.4f) * 2.5f), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
		//view = glm::lookAt(glm::vec3(3.0f, 0.0f, 0.0f), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

		glm::mat4 projView = proj * view;


		renderer.render(server, planet, projView);
		
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(window);
		glfwPollEvents();

		dt = (float)dtt.restart();
		t += dt;
	}

	logger->info("Ending OSP");

	glfwDestroyWindow(window);
	glfwTerminate();

	destroyGlobalAssetManager();
	destroyGlobalLogger();

}