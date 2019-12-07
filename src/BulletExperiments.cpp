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

InputUtil* input;

btRigidBody* floor_body;
btMotionState* floor_state;
glm::dvec3 base_speed;


void test(btDynamicsWorld* world, btScalar timeStep)
{
	btTransform tform;
	//btTransform tform = floor_body->getWorldTransform();
	//floor_state->getWorldTransform(tform);
	//tform.getOrigin() += to_btVector3(base_speed * (double)timeStep);
	//floor_body->setWorldTransform(tform);
	//floor_state->setWorldTransform(tform);
	tform = floor_body->getWorldTransform();

	floor_body->setLinearVelocity(to_btVector3(base_speed));
	floor_body->setAngularVelocity(btVector3(0.0, 0.0, 0.0));
	floor_body->setWorldTransform(tform);
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

	glm::dvec3 base = glm::dvec3(AU * 10.0, 0.0, 0.0);
	base_speed = glm::dvec3(100000.0, 0.0, 0.0);

	SimpleCamera camera = SimpleCamera();
	camera.pos = base + glm::dvec3(-40.0f, 0.0f, -20.0f);
	camera.fw = glm::normalize(glm::dvec3(1.0f, 0.0f, 0.5f));

	btDefaultCollisionConfiguration* collision_config = new btDefaultCollisionConfiguration();
	btCollisionDispatcher* dispatcher = new btCollisionDispatcher(collision_config);
	btBroadphaseInterface* brf_interface = new btDbvtBroadphase();
	btSequentialImpulseConstraintSolver* solver = new btSequentialImpulseConstraintSolver();
	btDiscreteDynamicsWorld* world = new btDiscreteDynamicsWorld(dispatcher, brf_interface, solver, collision_config);


	BulletDebugDrawer* bullet_debug_drawer = new BulletDebugDrawer();
	world->setDebugDrawer(bullet_debug_drawer);

	bullet_debug_drawer->setDebugMode(
		btIDebugDraw::DBG_DrawConstraints |
		btIDebugDraw::DBG_DrawContactPoints |
		btIDebugDraw::DBG_DrawWireframe);

	world->setGravity(btVector3(0.0, -10.0, 0.0));


	btRigidBody* a;
	btRigidBody* b;
	btRigidBody* c;

	world->setInternalTickCallback(test);

	{
		btCollisionShape* groundShape = new btBoxShape(btVector3(btScalar(50.), btScalar(50.), btScalar(50.)));

		btTransform groundTransform;
		groundTransform.setIdentity();
		groundTransform.setOrigin(to_btVector3(base + glm::dvec3(0.0, -56.0, 0.0)));



		btScalar mass(100000000000.0);

		//rigidbody is dynamic if and only if mass is non zero, otherwise static
		bool isDynamic = (mass != 0.f);

		btVector3 localInertia(0, 0, 0);
		if (isDynamic)
			groundShape->calculateLocalInertia(mass, localInertia);

		//using motionstate is optional, it provides interpolation capabilities, and only synchronizes 'active' objects
		btDefaultMotionState* myMotionState = new btDefaultMotionState(groundTransform);
		btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, groundShape, localInertia);
		btRigidBody* body = new btRigidBody(rbInfo);

		//body->setCollisionFlags(body->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
		//body->setActivationState(DISABLE_DEACTIVATION);

		//add the body to the dynamics world
		world->addRigidBody(body);
		floor_body = body;
		floor_state = myMotionState;

		floor_body->setLinearVelocity(to_btVector3(base_speed));
		floor_body->setGravity(btVector3(0.0, 0.0, 0.0));
	}

	{
		//create a dynamic rigidbody

		//btCollisionShape* colShape = new btBoxShape(btVector3(1,1,1));
		btCollisionShape* colShape = new btSphereShape(btScalar(1.));

		/// Create Dynamic Objects
		btTransform startTransform;
		startTransform.setIdentity();

		btScalar mass(4.f);

		//rigidbody is dynamic if and only if mass is non zero, otherwise static
		bool isDynamic = (mass != 0.f);

		btVector3 localInertia(0, 0, 0);
		if (isDynamic)
			colShape->calculateLocalInertia(mass, localInertia);

		startTransform.setOrigin(to_btVector3(base + glm::dvec3(4, 14, 0)));

		//using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
		btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);
		btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, colShape, localInertia);
		btRigidBody* body = new btRigidBody(rbInfo);

		world->addRigidBody(body);
		body->setLinearVelocity(to_btVector3(base_speed));

		a = body;
	}

	{
		//create a dynamic rigidbody

		//btCollisionShape* colShape = new btBoxShape(btVector3(1,1,1));
		btCollisionShape* colShape = new btSphereShape(btScalar(1.));

		/// Create Dynamic Objects
		btTransform startTransform;
		startTransform.setIdentity();

		btScalar mass(4.f);

		//rigidbody is dynamic if and only if mass is non zero, otherwise static
		bool isDynamic = (mass != 0.f);

		btVector3 localInertia(0, 0, 0);
		if (isDynamic)
			colShape->calculateLocalInertia(mass, localInertia);

		startTransform.setOrigin(to_btVector3(base + glm::dvec3(4, 6, 0)));

		//using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
		btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);
		btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, colShape, localInertia);
		btRigidBody* body = new btRigidBody(rbInfo);

		world->addRigidBody(body);
		body->setLinearVelocity(to_btVector3(base_speed));

		c = body;
	}

	{
		//create a dynamic rigidbody

		//btCollisionShape* colShape = new btBoxShape(btVector3(1,1,1));
		btCollisionShape* colShape = new btBoxShape(to_btVector3(glm::dvec3(2.0, 2.0, 2.0)));

		/// Create Dynamic Objects
		btTransform startTransform;
		startTransform.setIdentity();

		btScalar mass(1.f);

		//rigidbody is dynamic if and only if mass is non zero, otherwise static
		bool isDynamic = (mass != 0.f);

		btVector3 localInertia(0, 0, 0);
		if (isDynamic)
			colShape->calculateLocalInertia(mass, localInertia);

		startTransform.setOrigin(to_btVector3(base + glm::dvec3(1, 10, 0)));

		//using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
		btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);
		btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, colShape, localInertia);
		btRigidBody* body = new btRigidBody(rbInfo);

		world->addRigidBody(body);
		body->setLinearVelocity(to_btVector3(base_speed));

		b = body;
	}

	btPoint2PointConstraint* link = new btPoint2PointConstraint(*a, *b, btVector3(0.0, -7.0, 0.0), btVector3(0.0, -2.0, 0.0));
	btPoint2PointConstraint* link2 = new btPoint2PointConstraint(*a, *c, btVector3(0.0, -7.0, 0.0), btVector3(0.0, 2.0, 0.0));

	world->addConstraint(link);
	world->addConstraint(link2);

	while (!glfwWindowShouldClose(renderer.window))
	{
		input->update(renderer.window);


		glfwPollEvents();


		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();



		double step = 1.0 / 60.0;
		int sub_steps = world->stepSimulation(dt, 10);

		// AGGHH this is neccesary :(
		camera.pos += base_speed * (double)sub_steps * step;
		btTransform tform;

		// SECOND ARGUMENT HAS TO BE ZERO, OTHERWISE IT BREAKS!
		// (Because we change the floor position while the engine
		// may have a different dt, resulting in jumps)
		// TODO: Find a way to allow substeps

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



	}

	logger->info("Ending OSP");

	delete input;

	destroy_global_text_drawer();
	destroy_global_texture_drawer();
	destroy_global_asset_manager();
	destroy_global_logger();


}