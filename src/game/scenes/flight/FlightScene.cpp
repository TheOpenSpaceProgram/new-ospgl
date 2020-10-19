#include "FlightScene.h"
#include <OSP.h>
#include <universe/Universe.h>

#include <universe/vehicle/Vehicle.h>
#include <universe/vehicle/VehicleLoader.h>
#include <universe/entity/entities/VehicleEntity.h>
#include <universe/entity/entities/BuildingEntity.h>

void FlightScene::load()
{
	game_state = &get_osp()->game_state;
	universe = &game_state->universe;
	
	debug_drawer->debug_enabled = true;

	// Add all entities and system to the renderer
	get_osp()->renderer->add_drawable(&universe->system);
	
	for(Entity* ent : universe->entities)
	{
		get_osp()->renderer->add_drawable(ent);
	}

	// Sign up for new entities to automatically add them to the renderer
	universe->sign_up_for_event("core:new_entity", EventHandler([](EventArguments& args, const void* self)
	{
		FlightScene* self_s = (FlightScene*)self;
		int64_t id = std::get<int64_t>(args[0]);
		self_s->get_osp()->renderer->add_drawable(self_s->universe->get_entity(id));	

	}, this));

	// And automatically remove them when they die
	universe->sign_up_for_event("core:remove_entity", EventHandler([](EventArguments& args, const void* self)
	{
		FlightScene* self_s = (FlightScene*)self;
		int64_t id = std::get<int64_t>(args[0]);
		self_s->get_osp()->renderer->remove_drawable(self_s->universe->get_entity(id));

	}, this));

	get_osp()->renderer->cam = &camera;
	camera.speed = 20.0;


	Vehicle* n_vehicle = new Vehicle();
	SerializeUtil::read_file_to("udata/vehicles/debug.toml", *n_vehicle);

	get_osp()->game_state.universe.create_entity<VehicleEntity>(n_vehicle);

	WorldState st = WorldState();
	BuildingEntity* lpad_ent = (BuildingEntity*)get_osp()->game_state.universe.entities[0];
	WorldState stt = lpad_ent->traj.get_state(0.0, true);

	st.cartesian.pos = stt.cartesian.pos;
	st.cartesian.vel = stt.cartesian.vel;
	st.rotation = stt.rotation;
	st.cartesian.pos += stt.rotation * glm::dvec3(0, 0, 1) * 10.0;

	n_vehicle->packed_veh.set_world_state(st);
	n_vehicle->unpack();

	// Pass control to the capsule (root)
	Machine* capsule = n_vehicle->root->part->get_machine("capsule");	
	auto result = LuaUtil::call_function_if_present(capsule->env["get_input_context"], "Flight Scene obtain input context");
	if(result.has_value() && result->valid())
	{
		input.set_ctx(result->get<InputContext*>());		
	}	

	Renderer* r = get_osp()->renderer;

	r->add_drawable(&sky);
	r->set_ibl_source(nullptr);


	sun = SunLight(r->quality.sun_terrain_shadow_size, r->quality.sun_shadow_size);
	r->add_light(&sun);
}

void FlightScene::unload()
{

}

void FlightScene::update()
{
	gui_input.update();

	// GUI preparation goes here
	
	input.keyboard_blocked = camera.keyboard_blocked || gui_input.keyboard_blocked;
	input.update(get_osp()->renderer->window, get_osp()->game_dt);

	VehicleEntity* v_ent =  universe->get_entity_as<VehicleEntity>(2);	
	
	camera.center = v_ent->vehicle->unpacked_veh.get_center_of_mass(true);
	
	camera.update(get_osp()->game_dt);

}

void FlightScene::render()
{
	get_osp()->renderer->render(&get_osp()->game_state.universe.system);	
}
