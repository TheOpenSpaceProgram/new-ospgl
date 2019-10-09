#include "PlanetTileServer.h"
#include <imgui/imgui.h>


void PlanetTileServer::update(const QuadTreePlanet& planet)
{
	if (dirty)
	{
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

	std::vector<PlanetTilePath> paths = planet.get_all_leaf_paths();

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


PlanetTileServer::PlanetTileServer()
{
	threads_run = true;
	depth_for_unload = 0;

	for (size_t i = 0; i < threads.size(); i++)
	{
		threads[i] = new std::thread(thread_func, this);
	}
}


PlanetTileServer::~PlanetTileServer()
{
	threads_run = false;

	for (size_t i = 0; i < threads.size(); i++)
	{
		// We must work hard to get those threads to wake up!
		condition_var.notify_all();

		threads[i]->join();
		delete threads[i];
	}

}

void PlanetTileServer::do_imgui()
{
	// (Not really unsafe!)
	size_t tiles_size = tiles.get_unsafe()->size();
	ImGui::Text("Loaded tiles: %i (%.2fMB)", tiles_size, (float)(tiles_size * sizeof(PlanetTile)) / 1000000.0f);
	ImGui::Text("Work List: %i", work_list.get_unsafe()->size());
}

void PlanetTileServer::thread_func(PlanetTileServer* server)
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
			ntile->generate(target);

			{
				// Send it to the tiles
				auto tiles_w = server->tiles.get();

				(*tiles_w)[target] = ntile;

			}

			server->dirty = true;
		}
	}
}

