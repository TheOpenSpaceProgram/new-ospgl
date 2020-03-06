#include "PlanetTileServer.h"
#include <imgui/imgui.h>
#include "../../util/Logger.h"

void PlanetTileServer::update(QuadTreePlanet& planet)
{
	if (dirty)
	{
		planet.iteration++;
		auto tiles_w = tiles.get();

		for (auto it = tiles_w->begin(); it != tiles_w->end(); it++)
		{
			if (!it->second->is_uploaded())
			{
				it->second->upload();
			}
		}

		dirty = false;
	}

	if (!planet.dirty)
	{
		return;
	}

	std::vector<PlanetTilePath> paths = planet.get_all_paths();

	std::vector<PlanetTilePath> new_paths;
	{
		// We obtain the lock on tiles during this block
		auto tiles_w = tiles.get();

		// Find new paths to generate
		for (size_t i = 0; i < paths.size(); i++)
		{
			if (tiles_w->find(paths[i]) == tiles_w->end())
			{
				new_paths.push_back(paths[i]);
			}
		}

		// Find unused paths to unload, and unload them
		for (auto it = tiles_w->begin(); it != tiles_w->end();)
		{
			bool found = false;
			for(size_t i = 0; i < paths.size(); i++)
			{ 
				if (paths[i] == it->first)
				{
					found = true;
					break;
				}
			}

			if (!found)
			{
				if (it->first.get_depth() > depth_for_unload)
				{
					delete it->second;
					it = tiles_w->erase(it);
				}
				else
				{
					it++;
				}
			}
			else
			{


				it++;
			}
		}

	}

	{
		auto work_list_w = work_list.get();

		work_list_w->clear();

		for (size_t i = 0; i < new_paths.size(); i++)
		{
			work_list_w->insert(new_paths[i]);
		}

		if (work_list_w->size() != 0)
		{
			condition_var.notify_all();
		}
	}

}

void PlanetTileServer::set_depth_for_unload(int depth)
{
	depth_for_unload = depth;
}



double PlanetTileServer::get_height(glm::dvec3 pos_3d, size_t depth)
{
	pos_3d = glm::normalize(pos_3d);
	glm::dvec2 projected = MathUtil::euclidean_to_spherical_r1(pos_3d);


	default_lua(lua_state);

	PlanetTile::GeneratorInfo info;
	info.depth = (int)depth;
	info.coord_3d = pos_3d;
	info.coord_2d = projected;
	info.radius = config->radius;

	PlanetTile::GeneratorOut out;

	sol::protected_function func = lua_state["generate"];
	auto result = func(info, &out);

	// We ignore errors here
	if (result.valid())
	{
		return out.height;
	}
	else
	{
		return 0.0;
	}
}

PlanetTileServer::PlanetTileServer(const std::string& script, const std::string& script_path, 
	PlanetConfig* config, bool has_water)
{
	this->has_water = has_water;

	this->config = config;
	has_errors = false;
	threads_run = true;
	depth_for_unload = 0;

	bool wrote_error = false;

	PlanetTile::prepare_lua(lua_state);
	LuaUtil::safe_lua(lua_state, script, wrote_error, script_path);

	for (size_t i = 0; i < threads.size(); i++)
	{

		threads[i].thread = new std::thread(thread_func, this, &threads[i]);
		PlanetTile::prepare_lua(threads[i].lua_state);
		
		LuaUtil::safe_lua(threads[i].lua_state, script, wrote_error, script_path);

		if (wrote_error)
		{
			has_errors = true;
		}
	}
}


PlanetTileServer::~PlanetTileServer()
{
	threads_run = false;


	for (size_t i = 0; i < threads.size(); i++)
	{
		// We must work hard to get those threads to wake up!
		condition_var.notify_all();

		threads[i].thread->join();
		delete threads[i].thread;
	}

	// Tiles are now only managed by us
	for (auto it = tiles.get_unsafe()->begin(); it != tiles.get_unsafe()->end(); it++)
	{
		delete it->second;
	}



}

void PlanetTileServer::do_imgui()
{
	// (Not really unsafe!)
	size_t tiles_size = tiles.get_unsafe()->size();
	ImGui::Text("Loaded tiles: %i (%.2fMB)", tiles_size, (float)(tiles_size * sizeof(PlanetTile)) / 1000000.0f);
	ImGui::Text("Work List: %i", work_list.get_unsafe()->size());
}

void PlanetTileServer::thread_func(PlanetTileServer* server, PlanetTileThread* thread)
{
	PlanetTile::VertexArray<PlanetTileVertex, PlanetTile::TILE_SIZE>* work_array = 
		new PlanetTile::VertexArray<PlanetTileVertex, PlanetTile::TILE_SIZE>();
	while (server->threads_run)
	{
		std::unique_lock<std::mutex> lock(server->condition_mtx);
		server->condition_var.wait(lock);

		while (server->work_list.get_unsafe()->size() != 0)
		{
			PlanetTilePath target = PlanetTilePath(std::vector<QuadTreeQuadrant>(), PX);

			{
				auto work_list_w = server->work_list.get();

				if (work_list_w->size() == 0)
				{
					// This can happen, very unlikely, tough
					break;
				}
			
				
				// Simpler method, starts from smallest tile
				target = *work_list_w->begin();
				work_list_w->erase(work_list_w->begin());
				
				
			}

			// Work on the target
			PlanetTile* ntile = new PlanetTile();
			bool has_errors = ntile->generate(target, server->config->radius, 
				thread->lua_state, server->has_water, work_array);

			if (has_errors)
			{
				server->has_errors = true;
			}


			{
				// Send it to the tiles
				auto tiles_w = server->tiles.get();
				if (tiles_w->find(target) == tiles_w->end())
				{
					(*tiles_w)[target] = ntile;
				}
				else
				{
					// Weird, but can happen, thread will have wasted some precious CPU
					// time. TODO: Try to reduce the frequency of this
					delete ntile;
				}
				

			}

			server->dirty = true;
		}
	}

	delete work_array;
}

void PlanetTileServer::default_lua(sol::state& lua_state)
{

}

