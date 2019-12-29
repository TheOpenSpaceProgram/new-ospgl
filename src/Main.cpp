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

	Timer dtt = Timer();
	double dt = 0.0;

	input = new InputUtil();
	input->setup(renderer.window);


	PlanetarySystem system;
	assets->get_from_path<Config>("rss:systems/system_test.toml")->read_to(system);

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
	Config* navball_config = assets->get<Config>("navball", "navball.toml");
	navball_config->read_to(navball);


	SystemPointer center_ptr = SystemPointer(&system, "Earth");
	SystemPointer secondary_ptr = SystemPointer(&system, "Moon");

	ReferenceFrame ref(center_ptr);
	ref.mode = ReferenceFrame::ROTATING;
	ref.center2 = secondary_ptr;


	system.camera = MapCamera(SystemPointer(&system, 0));


	while (!glfwWindowShouldClose(renderer.window))
	{
		input->update(renderer.window);


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

		renderer.prepare_draw();
		
		if (renderer.render_enabled)
		{

			system.render(renderer.get_width(), renderer.get_height());
			//editor.render(width, height);

			navball.draw_to_texture(system.vessels[0], ref);

			system.render_debug(renderer.get_width(), renderer.get_height());


			renderer.prepare_gui();


			navball.draw_to_screen({ renderer.get_width(), renderer.get_height() });

			auto font = assets->get<BitmapFont>("core", "fonts/fira_code_medium.fnt");

		}

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(renderer.window);

		dt = dtt.restart();


	}

	logger->info("Ending OSP");

	delete input;

	destroy_global_lua_core();
	destroy_global_text_drawer();
	destroy_global_texture_drawer();
	destroy_global_asset_manager();
	destroy_global_logger();


}