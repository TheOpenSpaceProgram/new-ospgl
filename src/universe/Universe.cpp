#include "Universe.h"

static void bullet_tick(btDynamicsWorld* world, btScalar tstep)
{
	Universe* uv = (Universe*)world->getWorldUserInfo();

	// Call the system physics tick
	uv->physics_update(tstep);
}


std::unordered_set<EventHandler, EventHandlerHasher>& Universe::index_event_receivers(const std::string& str)
{
	auto it = event_receivers.find(str);
	if (it == event_receivers.end())
	{
		event_receivers[str] = std::unordered_set<EventHandler, EventHandlerHasher>();
		return event_receivers[str];
	}
	else
	{
		return it->second;
	}
}

void Universe::emit_event(const std::string& event_id, EventArguments args)
{
	auto& rc = index_event_receivers(event_id);

	for (EventHandler ev : rc)
	{
		ev.fnc(args, ev.user_data);
	}
}


void Universe::sign_up_for_event(const std::string& event_id, EventHandler id)
{
	auto& rc = index_event_receivers(event_id);
	rc.insert(id);
}


void Universe::drop_out_of_event(const std::string& event_id, EventHandler id)
{
	auto& rc = index_event_receivers(event_id);
	rc.erase(id);
}

void Universe::physics_update(double pdt)
{
	// Do the physics update on the system
	system.update(pdt, bt_world, true);

	for (Entity* e : entities)
	{
		e->physics_update(pdt);
	}
}

void Universe::update(double dt)
{
	system.update(dt, bt_world, false);

	bt_world->stepSimulation(dt, MAX_PHYSICS_STEPS, btScalar(PHYSICS_STEPSIZE));

	for (Entity* e : entities)
	{
		e->update(dt);
	}

	//bt_world->debugDrawWorld();
}

int64_t Universe::get_uid()
{
	// Increase BEFORE, uid=0 is the "nullptr" of ids
	uid++;
	return uid;
}

Entity* Universe::get_entity(int64_t uid)
{
	auto it = entities_by_id.find(uid);
	if(it == entities_by_id.end())
	{
		return nullptr;
	}

	return it->second;
}


Universe::Universe() : system(this)
{
	uid = 0;

	bt_collision_config = new btDefaultCollisionConfiguration();
	bt_dispatcher = new btCollisionDispatcher(bt_collision_config);
	bt_brf_interface = new btDbvtBroadphase();
	bt_solver = new btSequentialImpulseConstraintSolver();
	bt_world = new btDiscreteDynamicsWorld(bt_dispatcher, bt_brf_interface, bt_solver, bt_collision_config);
	
	bt_world->setGravity({ 0.0, 0.0, 0.0 });

	bt_debug = new BulletDebugDrawer();
	bt_world->setDebugDrawer(bt_debug);


	bt_debug->setDebugMode(
		btIDebugDraw::DBG_DrawConstraints |
		btIDebugDraw::DBG_DrawWireframe |
		btIDebugDraw::DBG_DrawFrames |
		btIDebugDraw::DBG_DrawConstraintLimits |
		btIDebugDraw::DBG_DrawAabb);
	

	bt_world->setInternalTickCallback(bullet_tick, this, true);

}


Universe::~Universe()
{
	for(Entity* ent : entities)
	{
		delete ent;
	}
}
