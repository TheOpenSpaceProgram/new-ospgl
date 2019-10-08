#include <sol.hpp>
#include <iostream>
#include "util/Logger.h"
#include "util/Timer.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

#include "planet_mesher/quadtree/QuadTreePlanet.h"
#include "planet_mesher/mesher/PlanetTileServer.h"


int main(void)
{
	createGlobalLogger();

	logger->info("Starting OSP");

	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	GLFWwindow* window = glfwCreateWindow(1500, 900, "New OSP", NULL, NULL);
	glfwMakeContextCurrent(window);
	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui::StyleColorsDark();

	float x = 0.2f;

	size_t depth = 3;

	QuadTreePlanet planet;
	planet.set_wanted_subdivide(glm::dvec2(x, 0.5), PX, depth);
	PlanetTileServer server;



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

			ImGui::End();
		}

		
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	logger->info("Ending OSP");

	glfwDestroyWindow(window);
	glfwTerminate();

	destroyGlobalLogger();

}