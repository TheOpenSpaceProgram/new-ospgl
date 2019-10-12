#include "PlanetTileServer.h"
#include <imgui/imgui.h>
#include "../../util/Logger.h"
#include <perlin.h>

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
				delete it->second;
				it = tiles_w->erase(it);
			}
			else
			{


				it++;
			}
		}

	}


	{
		// Push new paths to the work list, tiles is now unlocked, and we block work list
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

void safe_lua(sol::state& state, const std::string& script, bool& wrote_error)
{
	state.safe_script(script, [&wrote_error](lua_State*, sol::protected_function_result pfr)
	{

		if (!wrote_error)
		{
			sol::error err = pfr;
			logger->error("Lua Error:\n{}", err.what());
			wrote_error = true;
		}

		return pfr;
	});
}

double PlanetTileServer::get_height(glm::dvec3 pos_3d, size_t depth)
{
	pos_3d = glm::normalize(pos_3d);
	glm::dvec2 projected = MathUtil::euclidean_to_spherical_r1(pos_3d);

	lua_state["coord_3d"] = lua_state.create_table_with("x", pos_3d.x, "y", pos_3d.y, "z", pos_3d.z);
	lua_state["coord_2d"] = lua_state.create_table_with("x", projected.x, "y", projected.y);

	sol::protected_function func = lua_state["generate"];
	auto result = func();

	// We ignore errors here
	if (result.valid())
	{
		return result.get<double>();
	}
	else
	{
		return 0.0f;
	}
}

PlanetTileServer::PlanetTileServer(const std::string& script, PlanetMesherInfo* mesher_info)
{
	this->mesher_info = mesher_info;
	has_errors = false;
	threads_run = true;
	depth_for_unload = 0;

	bool wrote_error = false;

	prepare_lua(lua_state);
	safe_lua(lua_state, script, wrote_error);

	for (size_t i = 0; i < threads.size(); i++)
	{
		threads[i].thread = new std::thread(thread_func, this, &threads[i]);
		prepare_lua(threads[i].lua_state);
		
		safe_lua(threads[i].lua_state, script, wrote_error);

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

				target = *work_list_w->begin();
				work_list_w->erase(work_list_w->begin());
				
			}

			// Work on the target
			PlanetTile* ntile = new PlanetTile();
			bool has_errors = ntile->generate(target, server->mesher_info->radius, thread->lua_state);

			if (has_errors)
			{
				server->has_errors = true;
			}

			{
				// Send it to the tiles
				auto tiles_w = server->tiles.get();

				(*tiles_w)[target] = ntile;

			}

			server->dirty = true;
		}
	}
}

void PlanetTileServer::prepare_lua(sol::state& lua_state)
{
	lua_state.open_libraries(sol::lib::math, sol::lib::table, sol::lib::base);

	lua_state["perlin3d"] = [this](double x, double y, double z, int octaves) 
	{
		return this->noise.octaveNoise(x, y, z, octaves);
	};
}

