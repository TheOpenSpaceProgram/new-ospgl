#include "GameState.h"
#include <OSP.h>

GameState::GameState(OSP* n_osp) : universe()
{
	osp = n_osp;
}

void GameState::load(const cpptoml::table& from)
{
	std::string system_path = *from.get_as<std::string>("system");
	assets->get_from_path<Config>(system_path)->read_to(universe.system);

	double t0 = *from.get_as<double>("t");
	universe.system.t = t0;
	Date start_date = Date(t0);
	logger->info("Starting at: {}", start_date.to_string());

	universe.system.init(universe.bt_world);

	// These updates populate the element arrays
	universe.system.update(0.0, universe.bt_world, false);
	universe.system.update(0.0, universe.bt_world, true);

	// Load entities
	int64_t last_uid = *from.get_as<int64_t>("uid");
	universe.uid = last_uid;

	auto entities = from.get_table_array("entity");
	if (entities)
	{
		for (auto entity : *entities)
		{
			int64_t id = entity->get_as<int64_t>("id").value_or(0);
			if (id > last_uid || id <= 0)
			{
				logger->fatal("Invalid UID {} in save", id);
			}
			std::string type = *entity->get_as<std::string>("type");

			Entity* n_ent = Entity::load_entity(id, type, *entity);
			universe.entities.push_back(n_ent);
		}
	}

	// Init the universe entities (We have added them through special
	// code)

	std::sort(universe.entities.begin(), universe.entities.end(), [](Entity* a, Entity* b)
	{
		return a->get_uid() < b->get_uid();
	});

	for(Entity* ent : universe.entities)
	{
		ent->setup(&universe);
	}

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
	scene->setup(osp);
	scene->load();
}

