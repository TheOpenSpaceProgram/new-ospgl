#include "util/Logger.h"
#include "util/Timer.h"
#include "util/DebugDrawer.h"
#include "util/render/TextureDrawer.h"
#include "util/render/TextDrawer.h"
#include "renderer/Renderer.h"

#include "assets/Config.h"

#include "renderer/camera/SimpleCamera.h"

#include "util/InputUtil.h"
#include <imgui/imgui.h>

#include "universe/Date.h"

#include "vehicle/Vehicle.h"
#include "vehicle/part/link/SimpleLink.h"
#include "assets/Model.h"
#include "lua/LuaCore.h"

#include "physics/ground/GroundShape.h"
#include "renderer/lighting/SunLight.h"
#include "renderer/lighting/PointLight.h"

#include "universe/Universe.h"

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

	create_global_asset_manager();

	Renderer renderer = Renderer(*config);

	create_global_debug_drawer();
	create_global_texture_drawer();
	create_global_text_drawer();
	create_global_lua_core();

	{
		Timer dtt = Timer();
		double dt = 0.0;
		double t = 0.0;

		input = new InputUtil();
		input->setup(renderer.window);

		SimpleCamera* camera = new SimpleCamera();
		renderer.cam = camera;
		camera->speed = 10000000000.0;
		glm::dvec3 cam_offset = glm::dvec3(-10.0, 0.0f, 0.0f);
		camera->fw = glm::normalize(glm::dvec3(1.0f, 0.0f, 0.0));
		camera->pos = cam_offset;

		Universe universe = Universe(&renderer);
		assets->get_from_path<Config>("debug_system:systems/system.toml")->read_to(universe.system);

		debug_drawer->debug_enabled = true;

		//Date start_date = Date(2000, Date::MAY, 31);
		Date start_date = Date(2030, Date::AUGUST, 21);


		universe.system.t = start_date.to_seconds();
		logger->info("Starting at: {}", start_date.to_string());

		universe.system.init(universe.bt_world);

		universe.system.update(0.0, universe.bt_world, false);
		universe.system.update(0.0, universe.bt_world, true);



		SunLight sun = SunLight();
		renderer.add_light(&sun);

		while (!glfwWindowShouldClose(renderer.window))
		{
			input->update(renderer.window);

			glfwPollEvents();


			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();

			camera->update(dt);

			renderer.render();

			dt = dtt.restart();

			double max_dt = (double)(universe.MAX_PHYSICS_STEPS) * universe.PHYSICS_STEPSIZE;
			if (dt > max_dt)
			{
				logger->warn("Delta-time too high ({})/({}), slowing down", dt, max_dt);
				dt = max_dt;
			}

			dt = max_dt;
			t += dt;


		}

	}

	logger->info("Ending OSP");

	delete input;

	destroy_global_lua_core();
	destroy_global_text_drawer();
	destroy_global_texture_drawer();
	destroy_global_asset_manager();
	destroy_global_logger();


}
