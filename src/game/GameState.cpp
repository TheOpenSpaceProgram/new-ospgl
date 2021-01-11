#include "GameState.h"
#include <OSP.h>

GameState::GameState() : universe()
{
}

void GameState::load(const cpptoml::table& from)
{
	// Load the system
	universe.system.load(from);

	double t0 = *from.get_as<double>("t");
	universe.system.t0 = t0;
	Date start_date = Date(t0);
	logger->info("Starting at: {}", start_date.to_string());

	universe.system.init(universe.bt_world);

	// These updates populate the element arrays
	universe.system.update(0.0, universe.bt_world, false);
	universe.system.update(0.0, universe.bt_world, true);

	// Load entities
	int64_t last_uid = *from.get_as<int64_t>("uid");


	auto entities = from.get_table_array("entity");
	std::unordered_map<Entity*, int64_t> ent_to_id;

	if (entities)
	{
		for (const auto& entity : *entities)
		{
			int64_t id = entity->get_as<int64_t>("id").value_or(0);
			if (id > last_uid || id <= 0)
			{
				logger->fatal("Invalid UID {} in save", id);
			}
			std::string type = *entity->get_as<std::string>("type");

			Entity* n_ent = Entity::load_entity(type, *entity);
			universe.entities.push_back(n_ent);

			ent_to_id[n_ent] = id;
		}
	}

	// Init the universe entities (We have added them through special
	// code)

	// Init the hashtable
	for(Entity* ent : universe.entities)
	{
		universe.entities_by_id[ent_to_id[ent]] = ent;
	}

	// Init the entities
	for(Entity* ent : universe.entities)
	{
		ent->setup(&universe, ent_to_id[ent]);
	}

	universe.uid = last_uid;

	scene = nullptr;
	to_delete = nullptr;

}

void GameState::write(cpptoml::table& target) const
{

}

void GameState::update()
{
	if(to_delete != nullptr)
	{
		delete to_delete;
		to_delete = nullptr;
	}
	
	universe.update(osp->dt);
	
	if(scene)
	{
		scene->update();
	}


}

void GameState::render()
{
	if(scene)
	{
		scene->render();
	}
}


void GameState::load_scene(Scene* n_scene)
{
	if(scene != nullptr)
	{
		scene->unload();
		to_delete = scene;
	}

	scene = n_scene;
	scene->load();
}

