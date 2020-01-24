#include "PartViewer.h"
#include "../../util/DebugDrawer.h"


void PartViewer::update(double dt)
{
	t += dt;
}

void PartViewer::render(glm::ivec2 win_size)
{
	glm::dmat4 model = glm::dmat4(1.0);

	camera.pos = glm::dvec3(sin(t), 0.0, cos(t)) * 4.0;
	camera.fw = -camera.pos;

	CameraUniforms c_uniforms = camera.get_camera_uniforms(win_size.x, win_size.y);
	for (auto it = part->pieces.begin(); it != part->pieces.end(); it++)
	{
		it->second.model_node->draw(c_uniforms, model);

	}

	debug_drawer->add_line(glm::dvec3(0.0, 0.1, 0.5), glm::dvec3(10.0, 0.0, 0.0), glm::vec3(1.0, 0.0, 1.0));

	world->debugDrawWorld();

	debug_drawer->render(c_uniforms.proj_view, c_uniforms.c_model, c_uniforms.far_plane);
}

PartViewer::PartViewer(const std::string& part_path)
{
	part = AssetHandle<PartPrototype>(part_path);


	collision_config = new btDefaultCollisionConfiguration();
	dispatcher = new btCollisionDispatcher(collision_config);
	brf_interface = new btDbvtBroadphase();
	solver = new btSequentialImpulseConstraintSolver();
	world = new btDiscreteDynamicsWorld(dispatcher, brf_interface, solver, collision_config);

	world->setGravity({ 0.0, 0.0, 0.0 });

	bullet_debug_drawer = new BulletDebugDrawer();
	world->setDebugDrawer(bullet_debug_drawer);

	bullet_debug_drawer->setDebugMode(
		btIDebugDraw::DBG_DrawConstraints |
		btIDebugDraw::DBG_DrawWireframe |
		btIDebugDraw::DBG_DrawFrames |
		btIDebugDraw::DBG_DrawConstraintLimits);

	// Create rigidbodies so they render
	for (auto it = part->pieces.begin(); it != part->pieces.end(); it++)
	{
		PiecePrototype& piece = it->second;

		if (piece.collider)
		{
			btTransform tform = piece.collider_offset;

			btVector3 local_inertia;
			piece.collider->calculateLocalInertia(1.0, local_inertia);

			btMotionState* motion_state = new btDefaultMotionState(tform);
			btRigidBody::btRigidBodyConstructionInfo info(1.0, motion_state, piece.collider, local_inertia);
			btRigidBody* rigid_body = new btRigidBody(info);

			world->addRigidBody(rigid_body);
			bodies.push_back(rigid_body);
		}
	}

}


PartViewer::~PartViewer()
{
}
