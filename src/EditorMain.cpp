#include <sol.hpp>
#include <iostream>
#include "util/Logger.h"
#include "util/Timer.h"
#include "util/DebugDrawer.h"
#include "util/render/TextureDrawer.h"
#include "util/render/TextDrawer.h"
#include "renderer/Renderer.h"

#include "game/ui/Navball.h"
#include "assets/Config.h"


#include "util/InputUtil.h"
#include <imgui/imgui.h>


#include "tools/planet_editor/PlanetEditor.h"
#include "universe/PlanetarySystem.h"
#include "universe/Date.h"

#include "assets/BitmapFont.h"

#include "lua/LuaCore.h"


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
		Timer dtt = Timer();
		double dt = 0.0;

		input = new InputUtil();
		input->setup(renderer.window);


		Navball navball;
		Config* navball_config = assets->get<Config>("navball", "navball.toml");
		navball_config->read_to(navball);

		PlanetEditor editor = PlanetEditor(renderer.window, "rss:planets/earth/config.toml");


		while (!glfwWindowShouldClose(renderer.window))
		{
			input->update(renderer.window);


			glfwPollEvents();


			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();



			editor.update((float)dt);

			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

			renderer.prepare_draw();

			if (renderer.render_enabled)
			{

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