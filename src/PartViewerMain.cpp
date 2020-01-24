#include <sol.hpp>
#include <iostream>
#include "util/Logger.h"
#include "util/DebugDrawer.h"
#include "util/render/TextDrawer.h"
#include "util/Timer.h"
#include "renderer/Renderer.h"

#include "assets/Config.h"

#include "util/InputUtil.h"
#include <imgui/imgui.h>

#include "lua/LuaCore.h"

#include "tools/part_viewer/PartViewer.h"

InputUtil* input;


int main(void)
{

	create_global_logger();

	logger->info("Starting OSP");

	std::shared_ptr<cpptoml::table> config = SerializeUtil::load_file("settings.toml");

	if (!config)
	{
		logger->fatal("Could not find config file!");
	}

	Renderer renderer = Renderer(*config);

	create_global_asset_manager();
	create_global_debug_drawer();
	create_global_texture_drawer();
	create_global_text_drawer();
	create_global_lua_core();

	{

		PartViewer part_viewer = PartViewer("test_parts://parts/engine/part_engine.toml");

		Timer dtt = Timer();
		double dt = 0.0;

		input = new InputUtil();
		input->setup(renderer.window);

		while (!glfwWindowShouldClose(renderer.window))
		{
			input->update(renderer.window);


			glfwPollEvents();


			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();

			part_viewer.update(dt);

			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

			renderer.prepare_draw();

			if (renderer.render_enabled)
			{
				part_viewer.render(renderer.get_size());
				renderer.prepare_gui();
			}

			ImGui::Render();
			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

			glfwSwapBuffers(renderer.window);

			dt = dtt.restart();


		}

		logger->info("Ending OSP");

		delete input;

	}

	destroy_global_lua_core();
	destroy_global_text_drawer();
	destroy_global_texture_drawer();
	destroy_global_asset_manager();
	destroy_global_logger();


}