#include "util/Timer.h"

#include "renderer/camera/SimpleCamera.h"

#include <imgui/imgui.h>

#include "universe/Date.h"

#include "vehicle/Vehicle.h"
#include "vehicle/part/link/SimpleLink.h"
#include "assets/Model.h"

#include "physics/ground/GroundShape.h"
#include "renderer/lighting/SunLight.h"
#include "renderer/lighting/PointLight.h"

#include "universe/Universe.h"
#include "OSP.h"



int main(int argc, char** argv)
{
	OSP osp = OSP();
	osp.init(argc, argv);

	Timer dtt = Timer();
	double dt = 0.0;
	double t = 0.0;


	SimpleCamera* camera = new SimpleCamera();
	osp.renderer->cam = camera;
	camera->speed = 10000000000.0;
	glm::dvec3 cam_offset = glm::dvec3(-10.0, 0.0f, 0.0f);
	camera->fw = glm::normalize(glm::dvec3(1.0f, 0.0f, 0.0));
	camera->pos = cam_offset;
	camera->fov = 60.0f;

	Universe universe = Universe(osp.renderer);
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
	osp.renderer->add_light(&sun);

	while (!glfwWindowShouldClose(osp.renderer->window))
	{
		input->update(osp.renderer->window);

		glfwPollEvents();


		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		camera->update(dt);

		osp.renderer->render();

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

	osp.finish();
}
