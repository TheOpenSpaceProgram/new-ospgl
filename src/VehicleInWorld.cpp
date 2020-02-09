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

#include "universe/PlanetarySystem.h"
#include "universe/Date.h"

#include "vehicle/Vehicle.h"
#include "vehicle/part/link/SimpleLink.h"
#include "assets/Model.h"
#include "lua/LuaCore.h"


InputUtil* input;

void update_vehicles(std::vector<Vehicle*>& vehicles)
{
	for (Vehicle* v : vehicles)
	{
		v->draw_debug();
		auto n = v->update();
		vehicles.insert(vehicles.end(), n.begin(), n.end());
	}
}

btVector3 pos;
btVector3 vel;
btRigidBody* rigid;
btMotionState* state;
double sst = 0.0;
double tistep = 0.0;

class CustomMotionState : public btMotionState
{
public:

	virtual void getWorldTransform(btTransform& worldTrans) const
	{

		worldTrans.setIdentity();
		worldTrans.setOrigin(pos + vel * sst);

	}

	//Bullet only calls the update of worldtransform for active objects
	virtual void setWorldTransform(const btTransform& worldTrans)
	{

	}
};


void callback(btDynamicsWorld* world, btScalar tstep)
{
	btTransform p1 = btTransform::getIdentity();
	p1.setOrigin(pos + vel * sst);
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

		double AU = 149597900000.0;

		SimpleCamera camera = SimpleCamera();
		camera.speed = 100.0;
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
		p_engine.welded = false;

		v->dirty = true;


		update_vehicles(vehicles);


		PlanetarySystem system;
		assets->get_from_path<Config>("debug_system:systems/system.toml")->read_to(system);

		system.compute_sois(0.0);
		debug_drawer->debug_enabled = true;

		//Date start_date = Date(2000, Date::MAY, 31);
		Date start_date = Date(2020, Date::JUNE, 21);

		start_date.day_decimal = (19.0 + 27.0 / 60.0) / 24.0;

		system.t = start_date.to_seconds();
		logger->info("Starting at: {}", start_date.to_string());

		system.init(world);

		system.update(0.0, world);

		glm::dvec3 vvvel = system.states_now[3].vel;

		v->set_position(system.states_now[3].pos + glm::dvec3(6361000.0, 0.0, 0.0));
		v->set_linear_velocity(vvvel);
		//v->set_position(glm::dvec3(10.0, 0.0, 0.0));

		p_engine.rigid_body->applyImpulse(btVector3(0.0, 0.0, 1.0) * 100.0, btVector3(1.0, 0.0, 0.0));
		
		btTriangleMesh* mesh = new btTriangleMesh();
		mesh->addTriangle(btVector3(0.0, -50.0, 0.0), btVector3(0.0, 50.0, 50.0), btVector3(0.0, 50.0, -50.0));

		//CustomMotionState* st = new CustomMotionState();
		btDefaultMotionState* st = new btDefaultMotionState();
		btBvhTriangleMeshShape* plane = new btBvhTriangleMeshShape((btStridingMeshInterface*)mesh, true);
		//btBoxShape* plane = new btBoxShape(btVector3(1.0, 10.0, 10.0));
		btRigidBody* rg = new btRigidBody(0.0, nullptr, plane, btVector3(0.0, 0.0, 0.0));
		rg->setCollisionFlags(rg->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
		rg->setActivationState(DISABLE_DEACTIVATION);

		btTransform p1 = btTransform::getIdentity();
		btVector3 ppos = to_btVector3(system.states_now[3].pos + glm::dvec3(6361000.0, 0.0, 0.0) + glm::dvec3(10.0, 0.0, 0.0));
		btVector3 base_vel = to_btVector3(vvvel);
		p1.setOrigin(pos);
		rg->setWorldTransform(p1);

		world->addRigidBody(rg);


		world->setInternalTickCallback(callback, nullptr, true);

		rigid = rg;
		pos = ppos;
		vel = base_vel;
		sst = t;
		state = st;

		/*p_engine.rigid_body->setCcdMotionThreshold(0.0001);
		p_engine.rigid_body->setCcdSweptSphereRadius(0.5);
		p_capsule.rigid_body->setCcdMotionThreshold(0.0001);
		p_capsule.rigid_body->setCcdSweptSphereRadius(0.5);*/

		const double step = 1.0 / 30.0;
		const int max_steps = 1;

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
			


			

			double sub_steps = (double)world->stepSimulation(dt, max_steps, btScalar(step));
			p_engine.rigid_body->setGravity(btVector3(9.0, 0.0, 0.0));
			p_capsule.rigid_body->setGravity(btVector3(9.0, 0.0, 0.0));

			v->set_breaking_enabled(t > 0.0);

			camera.center = to_dvec3(p_engine.get_global_transform().getOrigin());
			camera.update(dt);

			std::vector<Vehicle*> n_vehicles;

			update_vehicles(vehicles);

			system.update(dt, world);

			renderer.prepare_draw();


			CameraUniforms c_uniforms = camera.get_camera_uniforms(renderer.get_width(), renderer.get_height());

			glm::dmat4 proj_view = c_uniforms.proj_view;
			glm::dmat4 c_model = c_uniforms.c_model;
			float far_plane = c_uniforms.far_plane;

			glm::dvec3 p = to_dvec3(pos + vel * t);

			debug_drawer->add_line(
				to_dvec3(p_engine.get_global_transform().getOrigin()),
				p, glm::vec3(1.0, 0.0, 1.0));

			if (renderer.render_enabled)
			{
				world->debugDrawWorld();

				system.render(renderer.get_width(), renderer.get_height(), c_uniforms);

				for (Vehicle* v : vehicles)
				{
					v->render(c_uniforms, LightingUniforms());
				}

				system.render_debug(renderer.get_width(), renderer.get_height(), c_uniforms);

				debug_drawer->render(proj_view, c_model, far_plane);

				renderer.prepare_gui();

			}

			renderer.finish();

			dt = dtt.restart();

			double max_dt = (double)(max_steps) * step;
			if (dt > max_dt)
			{
				logger->warn("Delta-time too high ({})/({}), slowing down", dt, max_dt);
				dt = max_dt;
			}

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