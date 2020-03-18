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

#include "universe/PlanetarySystem.h"
#include "universe/Date.h"

#include "vehicle/Vehicle.h"
#include "vehicle/part/link/SimpleLink.h"
#include "assets/Model.h"
#include "lua/LuaCore.h"

#include "physics/ground/GroundShape.h"
#include "physics/debug/BulletDebugDrawer.h"
#include "renderer/lighting/SunLight.h"
#include "renderer/lighting/PointLight.h"

InputUtil* input;

void update_vehicles(std::vector<Vehicle*>& vehicles, Renderer& render)
{
	for (Vehicle* v : vehicles)
	{
		v->draw_debug();
		auto n = v->update();
		vehicles.insert(vehicles.end(), n.begin(), n.end());

		if (!v->is_in_renderer())
		{
			render.add_drawable(v, "vehicle");
		}
	}
}

PlanetarySystem* ssystem;
btRigidBody* rigid;
double sst = 0.0;



void callback(btDynamicsWorld* world, btScalar tstep)
{
	ssystem->update(tstep, world, true);


	btTransform p1 = btTransform::getIdentity();
	p1.setOrigin(to_btVector3(ssystem->bullet_states[3].pos));
	glm::dmat4 mat = ssystem->elements[3].as_body->build_rotation_matrix(ssystem->bt);
	glm::dquat quat = glm::dquat(mat);

	btQuaternion rot = to_btQuaternion(quat);

	p1.setRotation(rot);

	rigid->setWorldTransform(p1);
	//state->setWorldTransform(p1);

	//rigid->setLinearVelocity(vel);

	sst += tstep;

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

	create_global_asset_manager();

	Renderer renderer = Renderer(*config);

	create_global_debug_drawer();
	create_global_texture_drawer();
	create_global_text_drawer();
	create_global_lua_core();

	{
		Timer dtt = Timer();

		double dt = 0.0;

		input = new InputUtil();
		input->setup(renderer.window);

		double AU = 149597900000.0;

		SimpleCamera* camera = new SimpleCamera();
		renderer.cam = camera;
		camera->speed = 100.0;
		glm::dvec3 cam_offset = glm::dvec3(-10.0, 0.0f, 0.0f);
		camera->fw = glm::normalize(glm::dvec3(1.0f, 0.0f, 0.0));
		camera->pos = cam_offset;

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
			btIDebugDraw::DBG_DrawConstraintLimits |
			btIDebugDraw::DBG_DrawAabb);


		std::vector<Vehicle*> vehicles;

		double t = 0.0;


		AssetHandle<PartPrototype> capsule = AssetHandle<PartPrototype>("test_parts:parts/capsule/part_capsule.toml");
		AssetHandle<PartPrototype> engine = AssetHandle<PartPrototype>("test_parts:parts/engine/part_engine.toml");

		Vehicle* v = new Vehicle(world);
		vehicles.push_back(v);

		Piece p_capsule = Piece(capsule.duplicate(), "p_root");
		Piece p_engine = Piece(engine.duplicate(), "p_root");

		v->add_piece(&p_capsule, btTransform::getIdentity());
		btTransform enginet = btTransform::getIdentity();
		enginet.setOrigin(btVector3(0.0, 0.0, -2.00));
		v->add_piece(&p_engine, enginet);

		v->root = &p_capsule;

		p_engine.attached_to = &p_capsule;
		p_engine.link_from = btVector3(0.0, 0.0, 2.0 / 2.0);
		p_engine.link_to = btVector3(0.0, 0.0, -2.0 / 2.0);

		p_engine.link = std::make_unique<SimpleLink>(world);
		p_engine.welded = true;
		
		v->dirty = true;


		update_vehicles(vehicles, renderer);


		PlanetarySystem system;
		assets->get_from_path<Config>("debug_system:systems/system.toml")->read_to(system);

		renderer.add_drawable(&system, "system");

		system.compute_sois(0.0);
		debug_drawer->debug_enabled = true;

		//Date start_date = Date(2000, Date::MAY, 31);
		Date start_date = Date(2030, Date::AUGUST, 21);


		system.t = start_date.to_seconds();
		logger->info("Starting at: {}", start_date.to_string());

		system.init(world);

		system.update(0.0, world, false);
		system.update(0.0, world, true);

		glm::dvec3 vvvel = system.states_now[3].vel;

		v->set_position(system.states_now[3].pos + glm::dvec3(6361000.0, 0.0, 0.0));
		v->set_linear_velocity(vvvel);
		//v->set_position(glm::dvec3(10.0, 0.0, 0.0));

		//p_engine.rigid_body->applyImpulse(btVector3(0.0, 0.0, 1.0) * 100.0, btVector3(1.0, 0.0, 0.0));
		
		//btTriangleMesh* mesh = new btTriangleMesh();
		//mesh->addTriangle(btVector3(0.0, -50.0, 0.0), btVector3(0.0, 50.0, 50.0), btVector3(0.0, 50.0, -50.0));

		//CustomMotionState* st = new CustomMotionState();
		btDefaultMotionState* st = new btDefaultMotionState();
		//btBvhTriangleMeshShape* plane = new btBvhTriangleMeshShape((btStridingMeshInterface*)mesh, true);
		GroundShape* plane = new GroundShape(system.elements[3].as_body);
		//btBoxShape* plane = new btBoxShape(btVector3(1.0, 10.0, 10.0));
		btRigidBody* rg = new btRigidBody(10000000000.0, nullptr, plane, btVector3(0.0, 0.0, 0.0));
		rg->setCollisionFlags(rg->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
		rg->setActivationState(DISABLE_DEACTIVATION);

		rg->setRestitution(1.0);
		rg->setFriction(0.8);

		plane->setMargin(2.0);

		btTransform p1 = btTransform::getIdentity();
		p1.setOrigin(to_btVector3(system.states_now[3].pos));
		rg->setWorldTransform(p1);

		world->addRigidBody(rg);


		world->setInternalTickCallback(callback, nullptr, true);

		rigid = rg;
		sst = t;
		ssystem = &system;

		const double step = 1.0 / 30.0;
		const int max_steps = 1;
		
		p_engine.rigid_body->setGravity(btVector3(-9.0, 0.0, 0.0));
		p_capsule.rigid_body->setGravity(btVector3(-9.0, 0.0, 0.0));
	
		SunLight sun = SunLight();
		renderer.add_light(&sun);

		while (!glfwWindowShouldClose(renderer.window))
		{
			input->update(renderer.window);


			glfwPollEvents();


			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();


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

			if (glfwGetKey(renderer.window, GLFW_KEY_B) == GLFW_PRESS)
			{
				v->set_position(system.states_now[3].pos + glm::dvec3(6361000.0, 0.0, 4000.0));
			}

			if (glfwGetKey(renderer.window, GLFW_KEY_V) == GLFW_PRESS)
			{
				v->set_position(system.states_now[3].pos + glm::dvec3(6361000.0, 500.0, 0.0));
			}
			

			ssystem->update(dt, world, false);

			double sub_steps = (double)world->stepSimulation(dt, max_steps, btScalar(step));

			plane->cur_offset = system.states_now[3].pos;

			v->set_breaking_enabled(t > 0.0);

			camera->center = to_dvec3(p_engine.get_global_transform().getOrigin());

			camera->update(dt);

			std::vector<Vehicle*> n_vehicles;

			update_vehicles(vehicles, renderer);

			renderer.render();

			dt = dtt.restart();

			double max_dt = (double)(max_steps) * step;
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
