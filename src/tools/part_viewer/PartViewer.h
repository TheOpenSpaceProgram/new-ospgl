#pragma once
#include "../../vehicle/part/Part.h"
#include "../../util/FileWatcher.h"
#include "../../assets/PartPrototype.h"
#include "../../renderer/camera/SimpleCamera.h"
#pragma warning(push, 0)
#include <btBulletDynamicsCommon.h>
#include <BulletDynamics/ConstraintSolver/btNNCGConstraintSolver.h>
#pragma warning(pop)

#include "../../physics/debug/BulletDebugDrawer.h"

// Supports reloading of the part for quick design from the developer
class PartViewer
{
public:

	double t;

	btDefaultCollisionConfiguration* collision_config;
	btCollisionDispatcher* dispatcher;
	btBroadphaseInterface* brf_interface;
	btSequentialImpulseConstraintSolver* solver;
	btDiscreteDynamicsWorld* world;
	BulletDebugDrawer* bullet_debug_drawer;

	std::vector<btRigidBody*> bodies;

	AssetHandle<PartPrototype> part;

	FileWatcher model_watch;
	FileWatcher config_watch;

	SimpleCamera camera;

	void update(double dt);
	void render(glm::ivec2 win_size);

	PartViewer(const std::string& part_path);
	~PartViewer();
};

