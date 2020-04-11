#include "FlightScene.h"
#include <OSP.h>
#include <universe/Universe.h>

void FlightScene::load()
{
	Universe& universe = get_osp()->game_state.universe;

	debug_drawer->debug_enabled = true;

	// Add all entities and system to the renderer
	get_osp()->renderer->add_drawable(&universe.system);
	
	for(Entity* ent : universe.entities)
	{
		get_osp()->renderer->add_drawable(ent);
	}

	// Sign up for new entities to automatically add them to the renderer
	universe.sign_up_for_event("core:new_entity", EventHandler([](EventArguments& args, void* self)
	{
		FlightScene* self_s = (FlightScene*)self;

		logger->info("!");

	}, this));

	get_osp()->renderer->cam = &camera;


}

void FlightScene::unload()
{

}

void FlightScene::update()
{
	camera.update(get_osp()->game_dt);

}

void FlightScene::render()
{
	get_osp()->renderer->render(&get_osp()->game_state.universe.system);	
}
