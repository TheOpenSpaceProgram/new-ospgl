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
	double dt = 0.0;

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

	Vehicle veh = Vehicle(world);
	Piece* fuel = create_dummy_piece({ 0.0, -3.0, 0.0 }, &veh, 2.0);
	Piece* control = create_dummy_piece({ 0.0, 0.0, 0.0 }, &veh, 1.0);
	Piece* rad2 = create_dummy_piece({ -2.5, -3.0, 0.0 }, &veh, 0.5);
	Piece* rad = create_dummy_piece({ 2.5, -3.0, 0.0 }, &veh, 0.5);
	Piece* fuel3 = create_dummy_piece({ -5, -3.0, 0.0 }, &veh, 1.0);
	Piece* fuel2 = create_dummy_piece({ 5, -3.0, 0.0 }, &veh, 1.0);


	control->attached_to = nullptr;
	fuel->attached_to = control;
	rad->attached_to = fuel;
	rad2->attached_to = fuel;
	fuel2->attached_to = rad;
	fuel3->attached_to = rad2;

	fuel->welded = true;
	rad->welded = true;
	rad2->welded = true;
	fuel2->welded = true;
	fuel3->welded = true;

	veh.root = control;

	veh.sort();

	veh.build_physics();

	double t = 0.0;
	double pt = 0.0;

	veh.set_position(to_btVector3(base));
	veh.set_linear_velocity(to_btVector3(vbase));

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


		int sub_steps = world->stepSimulation(dt, max_steps, btScalar(step));

		double pdt = step * sub_steps;
		pt += pdt;

		if (glfwGetKey(input->window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
		{
			glm::dmat4 tform = glm::toMat4(to_dquat(fuel2->get_global_transform().getRotation()));
			glm::dvec3 f = tform * glm::dvec4(glm::dvec3(0.0, 1.0, 0.0) * 100.0, 1.0);
			glm::dmat4 tform2 = glm::toMat4(to_dquat(fuel3->get_global_transform().getRotation()));
			glm::dvec3 f2 = -tform2 * glm::dvec4(glm::dvec3(0.0, 1.0, 0.0) * 100.0, 1.0);

			glm::dvec3 pos = to_dvec3(fuel2->get_global_transform().getOrigin());
			glm::dvec3 pos2 = to_dvec3(fuel3->get_global_transform().getOrigin());

			fuel2->rigid_body->applyForce(to_btVector3(f), fuel2->get_relative_position());
			fuel3->rigid_body->applyForce(to_btVector3(f2), fuel3->get_relative_position());


			debug_drawer->add_cone(pos, pos - f, 1.0, glm::vec3(1.0, 1.0, 0.0));
			debug_drawer->add_cone(pos2, pos2 - f2, 1.0, glm::vec3(1.0, 1.0, 0.0));
		}

		if (glfwGetKey(input->window, GLFW_KEY_SPACE) == GLFW_PRESS)
		{
			rad->welded = false;
			rad2->welded = false;
			veh.build_physics();
		}


		glm::dvec3 fuel2_pos = to_dvec3(fuel2->get_global_transform().getOrigin());

		//cam_offset = glm::dvec3(sin(t * 0.5) * 30.0, -2.0, cos(t * 0.5) * 30.0);
		cam_offset = glm::dvec3(30.0, 0.0, 30.0);
		camera.pos = base + cam_offset + vbase * pt;// + fuel2_pos;
		camera.fw = glm::normalize((base + vbase * pt)-camera.pos);

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

		renderer.prepare_draw();

		debug_drawer->add_point(fuel2_pos, glm::vec3(1.0, 0.0, 1.0));
		veh.draw_debug();
	

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


	}

	logger->info("Ending OSP");

	delete input;

	destroy_global_text_drawer();
	destroy_global_texture_drawer();
	destroy_global_asset_manager();
	destroy_global_logger();


}