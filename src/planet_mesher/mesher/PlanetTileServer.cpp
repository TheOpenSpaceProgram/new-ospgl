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


	default_lua(lua_state, &noise);

	lua_state["coord_3d"]["x"] = pos_3d.x;
	lua_state["coord_3d"]["y"] = pos_3d.y;
	lua_state["coord_3d"]["z"] = pos_3d.z;

	lua_state["coord_2d"]["x"] = projected.x;
	lua_state["coord_2d"]["y"] = projected.y;

	lua_state["radius"] = config->radius;
	lua_state["depth"] = depth;

	sol::protected_function func = lua_state["generate"];
	auto result = func();

	// We ignore errors here
	if (result.valid())
	{
		return lua_state["height"].get_or(0.0);
	}
	else
	{
		return 0.0;
	}
}

PlanetTileServer::PlanetTileServer(const std::string& script, PlanetConfig* config, 
	int seed, int interp, bool has_water)
{
	this->has_water = has_water;

	noise_seed = seed;
	if (interp == 0)
	{
		noise_interp = FastNoise::Interp::Linear;
	}
	else if (interp == 1)
	{
		noise_interp = FastNoise::Interp::Hermite;
	}
	else
	{
		noise_interp = FastNoise::Interp::Quintic;
	}


	this->config = config;
	has_errors = false;
	threads_run = true;
	depth_for_unload = 0;

	bool wrote_error = false;


	auto image_paths = config->surface.image_paths;
	// Load all images
	for (auto it = image_paths.begin(); it != image_paths.end(); it++)
	{
		images.emplace(std::make_pair(it->first, it->second));
	}

	prepare_lua(lua_state, &noise);
	safe_lua(lua_state, script, wrote_error);
	noise = FastNoise();
	noise.SetSeed(seed);
	noise.SetInterp(noise_interp);

	for (size_t i = 0; i < threads.size(); i++)
	{
		threads[i].noise = FastNoise();
		threads[i].noise.SetInterp(noise_interp);
		threads[i].noise.SetSeed(seed);

		threads[i].thread = new std::thread(thread_func, this, &threads[i]);
		prepare_lua(threads[i].lua_state, &threads[i].noise);
		
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
	PlanetTile::VertexArray<PlanetTileVertex>* work_array = new PlanetTile::VertexArray<PlanetTileVertex>();
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
			bool has_errors = ntile->generate(target, server->config->radius, thread->lua_state, server->has_water,
				work_array, [&server, &thread]() {
				server->default_lua(thread->lua_state, &thread->noise);
			});

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

#include "../../util/lua/LuaNoiseLib.h"
#include "../../util/lua/LuaUtilLib.h"

void PlanetTileServer::prepare_lua(sol::state& lua_state, FastNoise* noise)
{
	lua_state.open_libraries(sol::lib::math, sol::lib::table, sol::lib::base);

	lua_state["coord_3d"] = lua_state.create_table_with("x", 0.0, "y", 0.0, "z", 0.0);
	lua_state["coord_2d"] = lua_state.create_table_with("x", 0.0, "y", 0.0);
	lua_state["color"] = lua_state.create_table_with("r", 0.0, "g", 0.0, "b", 0.0);
	lua_state["height"] = 0.0;

	LuaNoiseLib::load_lib(lua_state, noise);
	LuaUtilLib::load_lib(lua_state, config, images);

}

void PlanetTileServer::default_lua(sol::state & lua_state, FastNoise* noise)
{
	noise->SetFrequency(0.01);
	noise->SetFractalOctaves(3);
	noise->SetFractalLacunarity(2.0);
	noise->SetFractalGain(0.5);
	noise->SetFractalType(FastNoise::FBM);
	noise->SetCellularReturnType(FastNoise::CellValue);
}

