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
#include "vehicle/part/link/SimpleLink.h"

InputUtil* input;


#include "assets/Model.h"

#include "lua/LuaCore.h"


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
		Timer pdtt = Timer();

		double dt = 0.0;
		double pdt = 0.0;

		input = new InputUtil();
		input->setup(renderer.window);

		double AU = 149597900000.0;

		SimpleCamera camera = SimpleCamera();
		glm::dvec3 cam_offset = glm::dvec3(-10.0, 0.0f, 0.0f);
		camera.fw = glm::normalize(glm::dvec3(1.0f, 0.0f, 0.0));
		camera.pos = cam_offset;

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


		AssetHandle<PartPrototype> capsule = AssetHandle<PartPrototype>("test_parts:parts/capsule/part_capsule.toml");
		AssetHandle<PartPrototype> engine = AssetHandle<PartPrototype>("test_parts:parts/engine/part_engine.toml");

		Vehicle* v = new Vehicle(world);
		vehicles.push_back(v);

		Piece p_capsule = Piece(capsule.duplicate(), "p_root");
		Piece p_engine = Piece(engine.duplicate(), "p_root");
		Piece p_engine2 = Piece(engine.duplicate(), "p_root");

		v->add_piece(&p_capsule, btTransform::getIdentity());
		btTransform enginet = btTransform::getIdentity();
		enginet.setOrigin(btVector3(0.0, 0.0, -2.05));
		v->add_piece(&p_engine, enginet);
		enginet.setOrigin(btVector3(0.0, 2.1, -2.05));
		v->add_piece(&p_engine2, enginet);

		// Dont forget!
		v->root = &p_capsule;

		p_engine.attached_to = &p_capsule;
		p_engine.link_from = btVector3(0.0, 0.0, 2.0 / 2.0);
		p_engine.link_to = btVector3(0.0, 0.0, -2.0 / 2.0);

		p_engine.link = std::make_unique<SimpleLink>(world);
		p_engine.welded = false;

		p_engine2.attached_to = &p_engine;
		p_engine2.link_from = btVector3(0.0, -1.0, 0.0);
		p_engine2.link_to = btVector3(0.0, 1.0, 0.0);
		p_engine2.link = std::make_unique<SimpleLink>(world);
		p_engine2.welded = false;

		v->dirty = true;
		std::vector<Vehicle*> n_vehicles;

		for (Vehicle* v : vehicles)
		{
			v->draw_debug();
			auto n = v->update();
			n_vehicles.insert(n_vehicles.end(), n.begin(), n.end());


		}

		vehicles.insert(vehicles.end(), n_vehicles.begin(), n_vehicles.end());


		double force = 600.0;

	

		
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

			btVector3 origin = p_engine.get_local_transform() * btVector3(0.0, 0.0, 0.0);

			if (glfwGetKey(renderer.window, GLFW_KEY_L) == GLFW_PRESS)
			{
				p_engine.rigid_body->applyForce(btVector3(0.0, 0.0, force), origin);
			}

			if (glfwGetKey(renderer.window, GLFW_KEY_O) == GLFW_PRESS)
			{
				p_engine.rigid_body->applyForce(btVector3(0.0, 0.0, -force), origin);
			}

			if (glfwGetKey(renderer.window, GLFW_KEY_Y) == GLFW_PRESS)
			{
				p_engine.welded = false;
				p_engine.set_dirty();
			}


			if (glfwGetKey(renderer.window, GLFW_KEY_H) == GLFW_PRESS)
			{
				p_engine.welded = true;
				p_engine.set_dirty();
			}

			if (glfwGetKey(renderer.window, GLFW_KEY_I) == GLFW_PRESS)
			{
				force = 1000.0;
			}
			if (glfwGetKey(renderer.window, GLFW_KEY_K) == GLFW_PRESS)
			{
				force = 6000.0;
			}



			int sub_steps = world->stepSimulation(pdt, max_steps, btScalar(step));
			pdt = pdtt.restart();

			pt += pdt;

			v->set_breaking_enabled(pt > 0.5);


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

			camera.update(dt);
			CameraUniforms c_uniforms = camera.get_camera_uniforms(renderer.get_width(), renderer.get_height());

			glm::dmat4 proj_view = c_uniforms.proj_view;
			glm::dmat4 c_model = c_uniforms.c_model;
			float far_plane = c_uniforms.far_plane;

			if (renderer.render_enabled)
			{
				world->debugDrawWorld();

				glm::dmat4 capsule_tform = to_dmat4(p_capsule.get_global_transform());
				glm::dmat4 engine_tform = to_dmat4(p_engine.get_global_transform());

				glm::dmat4 capsule_rtform = glm::inverse(p_capsule.collider_offset);
				glm::dmat4 engine_rtform = glm::inverse(p_engine.collider_offset);

				glm::dmat4 capsule_ftform = capsule_rtform * capsule_tform;
				glm::dmat4 engine_ftform = engine_rtform * engine_tform;


				p_capsule.model_node->draw(c_uniforms, capsule_ftform, true);
				p_engine.model_node->draw(c_uniforms, engine_ftform, true);

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

	}

	logger->info("Ending OSP");

	delete input;

	destroy_global_lua_core();
	destroy_global_text_drawer();
	destroy_global_texture_drawer();
	destroy_global_asset_manager();
	destroy_global_logger();


}