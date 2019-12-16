#include <sol.hpp>
#include <iostream>
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

#pragma warning(push, 0)
#include <btBulletDynamicsCommon.h>
#include <BulletDynamics/ConstraintSolver/btNNCGConstraintSolver.h>
#pragma warning(pop)

#include "physics/debug/BulletDebugDrawer.h"

#include "vehicle/Vehicle.h"

InputUtil* input;


#include "assets/Model.h"



Piece* create_dummy_piece(glm::dvec3 offset, Vehicle* veh, double radius = 1.0)
{
	Piece* out = new Piece();

	btCollisionShape* colShape = new btBoxShape(btVector3(radius, radius, radius));

	out->collider = colShape;
	
	btTransform off_tform;
	off_tform.setIdentity();
	off_tform.setOrigin(to_btVector3(offset));

	out->mass = (4.0 / 3.0) * glm::pi<double>() * radius * radius * radius;

	veh->add_piece(out, off_tform);
	
	return out;
}


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

	Timer dtt = Timer();
	Timer pdtt = Timer();

	double dt = 0.0;
	double pdt = 0.0;

	input = new InputUtil();
	input->setup(renderer.window);

	double AU = 149597900000.0;

	glm::dvec3 base = glm::dvec3(1.0 * AU, 0.0, 0.0);
	glm::dvec3 vbase = glm::dvec3(40000.0, 0.0, 0.0);

	SimpleCamera camera = SimpleCamera();
	glm::dvec3 cam_offset = glm::dvec3(-10.0, 0.0f, -5.0f);
	camera.fw = glm::normalize(glm::dvec3(1.0f, 0.0f, 0.5f));

	btDefaultCollisionConfiguration* collision_config = new btDefaultCollisionConfiguration();
	btCollisionDispatcher* dispatcher = new btCollisionDispatcher(collision_config);
	btBroadphaseInterface* brf_interface = new btDbvtBroadphase();
	btSequentialImpulseConstraintSolver* solver = new btSequentialImpulseConstraintSolver();
	btDiscreteDynamicsWorld* world = new btDiscreteDynamicsWorld(dispatcher, brf_interface, solver, collision_config);

	world->setGravity({ 0.0, 0.0, 0.0 });

	BulletDebugDrawer* bullet_debug_drawer = new BulletDebugDrawer();
	world->setDebugDrawer(bullet_debug_drawer);

	bullet_debug_drawer->setDebugMode(
		btIDebugDraw::DBG_DrawConstraints |
		btIDebugDraw::DBG_DrawWireframe |
		btIDebugDraw::DBG_DrawFrames |
		btIDebugDraw::DBG_DrawConstraintLimits);


	std::vector<Vehicle*> vehicles;

	double pt = 0.0;
	double t = 0.0;


	Model* m = assets->get<Model>("assimp_test", "test/test.blend");

	while (!glfwWindowShouldClose(renderer.window))
	{
		input->update(renderer.window);


		glfwPollEvents();


		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();



		double step = 1.0 / 60.0;
		int max_steps = 32;

		double max_dt = (max_steps - 1) * step;

		if (dt > max_dt)
		{
			logger->warn("Delta-time too high ({})/({}), slowing down", dt, max_dt);
			dt = max_dt;
		}


		int sub_steps = world->stepSimulation(pdt, max_steps, btScalar(step));
		pdt = pdtt.restart();


		pt += pdt;


		std::vector<Vehicle*> n_vehicles;

		for (Vehicle* v : vehicles)
		{
			v->draw_debug();
			auto n = v->update();
			n_vehicles.insert(n_vehicles.end(), n.begin(), n.end());


		}

		vehicles.insert(vehicles.end(), n_vehicles.begin(), n_vehicles.end());

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

		renderer.prepare_draw();

		glm::dmat4 proj_view = camera.get_proj_view(renderer.get_width(), renderer.get_height());
		glm::dmat4 c_model = camera.get_cmodel();
		float far_plane = 1e16f;

		if (renderer.render_enabled)
		{
			world->debugDrawWorld();

			debug_drawer->render(proj_view, c_model, far_plane);

			renderer.prepare_gui();

		}

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());



		glfwSwapBuffers(renderer.window);

		dt = dtt.restart();
		t += dt;

		//logger->info("T: {} | PT: {}", t, pt);


	}

	logger->info("Ending OSP");

	delete input;

	destroy_global_text_drawer();
	destroy_global_texture_drawer();
	destroy_global_asset_manager();
	destroy_global_logger();


}