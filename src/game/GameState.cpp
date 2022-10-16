#include "GameState.h"
#include <game/scenes/editor/EditorScene.h>
#include <game/scenes/LuaScene.h>
#include <OSP.h>

GameState::GameState() : universe()
{
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

GameState* GameState::create_empty(const std::string &planetary_system_package)
{
	GameState* out = new GameState();

	out->used_packages.push_back("core");
	out->used_packages.push_back(planetary_system_package);

	return out;
}

GameState *GameState::load(const std::string &path)
{
	GameState* out = new GameState();
	// TODO: This could be moved somewhere else, but it's important
	// to run it quick, as otherwise lua environments get the wrong universe!
	osp->universe = &out->universe;

	out->to_delete = nullptr;
	out->scene = nullptr;
	out->path = path;
	logger->check(osp->assets->is_path_safe(path), "Unsafe path");

	std::string full_path = osp->assets->udata_path + "saves/" + path + "save.toml";

	auto root = SerializeUtil::load_file(full_path);
	// Read required packages
	auto pkgs = root->get_array_of<std::string>("packages");
	for(const std::string& pkg : *pkgs)
	{
		out->used_packages.push_back(pkg);
	}

	// Read the system
	out->universe.system.load(*root);

	double t0 = *root->get_as<double>("t");
	out->universe.system.t0 = t0;
	Date start_date = Date(t0);
	logger->info("Starting at: {}", start_date.to_string());

	out->universe.system.init(out->universe.bt_world);

	// These updates populate the element arrays
	out->universe.system.update(0.0, out->universe.bt_world, false);
	out->universe.system.update(0.0, out->universe.bt_world, true);

	// Load entities
	int64_t last_uid = *root->get_as<int64_t>("uid");


	auto entities = root->get_table_array("entity");
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

			Entity* n_ent = Entity::load(type, *entity);
			out->universe.entities.push_back(n_ent);

			out->ent_to_id[n_ent] = id;
		}
	}

	// Init the universe entities (We have added them through special
	// code)

	// Init the hashtable
	for(Entity* ent : out->universe.entities)
	{
		out->universe.entities_by_id[out->ent_to_id[ent]] = ent;
	}

	out->universe.uid = last_uid;

	// Finally, we may load the scene
	auto scene_toml = root->get_table("scene");
	const std::string scene_path = *scene_toml->get_as<std::string>("name");
	if(scene_path == "editor")
	{
		out->scene =new EditorScene();
	}
	else
	{
		// TODO: create the args
		std::vector<sol::object> args;
		auto* sc = new LuaScene(out, scene_path, "", args);
		out->scene = sc;
	}

	return out;
}

GameState *GameState::create_main_menu(const std::string &skip_to_save)
{
	if(skip_to_save.empty())
	{
		// Main menu proper TODO
		return load("debug-save/");
	}
	else
	{
		return load(skip_to_save);
	}
}

void GameState::init()
{
	// Init the entities
	for(Entity* ent : universe.entities)
	{
		ent->setup(&universe, ent_to_id[ent]);
	}

	scene->load();

}

