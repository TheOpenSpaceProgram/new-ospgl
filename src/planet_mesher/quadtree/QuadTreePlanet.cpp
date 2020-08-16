#include <cmath>

#include "QuadTreePlanet.h"
#include "imgui/imgui.h"

#include "../mesher/PlanetTileServer.h"
#include "../../util/Logger.h"

using namespace std;

void QuadTreePlanet::flatten()
{
	for (size_t i = 0; i < 6; i++)
	{
		sides[i].merge();
	}
}

std::vector<PlanetTilePath> QuadTreePlanet::get_all_render_leaf_paths(bool ignore_cache)
{
	if (iteration == old_render_leafs_it && !ignore_cache)
	{
		return old_render_leafs;
	}

	std::vector<PlanetTilePath> out;


	for (size_t i = 0; i < 6; i++)
	{
		std::vector<std::vector<QuadTreeQuadrant>> from_side = render_sides[i].get_all_leaf_paths();
		for (size_t j = 0; j < from_side.size(); j++)
		{
			out.push_back(PlanetTilePath(from_side[j], PlanetSide(i)));
		}
	}

	old_render_leafs = out;
	old_render_leafs_it = iteration;

	return out;
}

std::vector<QuadTreeNode*> QuadTreePlanet::get_all_leafs()
{
	std::vector<QuadTreeNode*> out;

	for (size_t i = 0; i < 6; i++)
	{
		std::vector<QuadTreeNode*> from_side = sides[i].get_all_leaf_nodes();
		out.insert(out.end(), from_side.begin(), from_side.end());
	}

	return out;
}

std::vector<PlanetTilePath> QuadTreePlanet::get_all_paths() const
{
	std::vector<PlanetTilePath> out;

	for (size_t i = 0; i < 6; i++)
	{
		std::vector<std::vector<QuadTreeQuadrant>> from_side = sides[i].get_all_paths();
		for (size_t j = 0; j < from_side.size(); j++)
		{
			out.push_back(PlanetTilePath(from_side[j], PlanetSide(i)));
		}
	}


	return out;
}

PlanetSide QuadTreePlanet::get_planet_side(glm::vec3 f) const
{
	float xabs = glm::abs(f.x);
	float yabs = glm::abs(f.y);
	float zabs = glm::abs(f.z);

	if (xabs >= yabs && xabs >= zabs)
	{
		return f.x >= 0.0f ? PX : NX;
	}

	if (yabs >= xabs && yabs >= zabs)
	{
		return f.y >= 0.0f ? PY :NY;
	}

	if (zabs >= xabs && zabs >= yabs)
	{
		return f.z >= 0.0f ? PZ : NZ;
	}


	return PX;
}

glm::dvec2 QuadTreePlanet::get_planet_side_offset(glm::vec3 point_normalized, PlanetSide side) const
{
	glm::dvec3 cube = MathUtil::sphere_to_cube(point_normalized);

	cube += glm::dvec3(1.0f, 1.0f, 1.0f);
	cube /= 2.0;

	if (isnan(cube.x))
	{
		cube.x = 0.5f;
	}

	if (isnan(cube.y))
	{
		cube.y = 0.5f;
	}

	if (isnan(cube.z))
	{
		cube.z = 0.5f;
	}

	if (side != NX && side != PY && side != NY)
	{
		cube.x = 1.0f - cube.x;
		cube.y = 1.0f - cube.y;
		cube.z = 1.0f - cube.z;
	}

	if (side == PZ)
	{
		cube.x = 1.0f - cube.x;
	}

	if (side == NY)
	{
		cube.x = 1.0f - cube.x;
	}

	if (side == PX || side == NX)
	{
		return glm::dvec2(cube.z, cube.y);
	}
	else if (side == PY || side == NY)
	{
		return glm::dvec2(cube.z, cube.x);
	}
	else
	{
		return glm::dvec2(cube.x, cube.y);
	}
}

void QuadTreePlanet::set_wanted_subdivide(glm::dvec2 offset, PlanetSide side, size_t depth)
{
	previous_depth = current_depth;

	dirty = true;

	if (current_depth > depth)
	{
		current_depth = depth;
	}

	wanted_pos = offset;
	wanted_side = side;
	wanted_depth = depth;
}


void QuadTreePlanet::update(PlanetTileServer& server)
{
	if (current_depth <= wanted_depth)
	{
		if (server.is_built())
		{
			flatten();


			sides[wanted_side].get_recursive(wanted_pos, current_depth);


			// Go for next step
			current_depth++;
			iteration++;

			dirty = true;
		}
	}

	for (size_t i = 0; i < 6; i++)
	{
		render_sides[i].merge();
	}

	render_sides[wanted_side].get_recursive(wanted_pos, current_depth - 1);

	// render_sides merges every parent of ANY NON LOADED CHILDREN
	for (size_t i = 0; i < 6; i++)
	{
		std::vector<QuadTreeNode*> all_leafs = render_sides[i].get_all_leaf_nodes();

		std::vector<QuadTreeNode*> to_merge;

		for (size_t j = 0; j < all_leafs.size(); j++)
		{
			PlanetTilePath path = PlanetTilePath(all_leafs[j]->get_path(), all_leafs[j]->planetside);
			bool found = true;
			{
				auto tiles_m = server.tiles.get();
				auto it = tiles_m->find(path);
				if (it == tiles_m->end())
				{
					found = false;
				}
			}

			if (!found)
			{
				if (path.path.size() != 0)
				{
					path.path.pop_back();
				}

				// path is now the parent
				QuadTreeNode* parent = render_sides[i].follow_path(path.path);
				bool good = true;

				// Check that renderer has parent, if it does not then we moved too far, reduce quality
				{
					auto tiles_m = server.tiles.get();
					if (tiles_m->find(path) == tiles_m->end())
					{
						// Horror, we moved too far, the user will be over low quality terrain
						good = false;
					}
				}

				if (good)
				{
					to_merge.push_back(parent);
				}
			}
		}

		// Make sure we only have parents in the list
		// otherwise we will try to merge nodes which
		// have been deleted (This can happen on very fast movement)
		// This ends up being quite fast, not only is to_merge
		// generally a small vector, but the is_children_of function
		// returns very early on most cases.
		// Don't bother removing this, it's the easy solution
		std::vector<int> to_ignore;
		to_ignore.resize(to_merge.size(), false);

		for (size_t i = 0; i < to_merge.size(); i++)
		{
			for (size_t j = 0; j < to_merge.size(); j++)
			{
				if (i != j)
				{
					if (to_merge[i]->is_children_of(to_merge[j]))
					{
						to_ignore[i] = true;
					}
				}
			}
		}

		for (size_t i = 0; i < to_merge.size(); i++)
		{
			if (to_ignore[i] == false)
			{
				to_merge[i]->merge();
			}
		}
	}
}

QuadTreeNode* QuadTreePlanet::subdivide_to(glm::dvec2 offset, PlanetSide side, size_t depth)
{
	return sides[side].get_recursive_simple(offset, depth);
}

QuadTreePlanet::QuadTreePlanet()
{
	iteration = 0;

	for (size_t i = 0; i < 6; i++)
	{
		sides[i] = QuadTreeNode();
	}

	current_depth = 0;
	previous_depth = 0;
	wanted_depth = 0;
	wanted_pos = glm::dvec2(0.5, 0.5);
	wanted_side = PX;

	// Cardinal directions have no meaning!
	sides[PX].neighbors[NORTH] = &sides[PY];
	sides[PX].neighbors[EAST] = &sides[NZ];
	sides[PX].neighbors[SOUTH] = &sides[NY];
	sides[PX].neighbors[WEST] = &sides[PZ];

	sides[NX].neighbors[NORTH] = &sides[NY];
	sides[NX].neighbors[EAST] = &sides[PZ];
	sides[NX].neighbors[SOUTH] = &sides[PY];
	sides[NX].neighbors[WEST] = &sides[NZ];

	sides[PY].neighbors[NORTH] = &sides[NX];
	sides[PY].neighbors[SOUTH] = &sides[PX];
	sides[PY].neighbors[EAST] = &sides[PZ];
	sides[PY].neighbors[WEST] = &sides[NZ];

	sides[PZ].neighbors[EAST] = &sides[PX];
	sides[PZ].neighbors[WEST] = &sides[NX];
	sides[PZ].neighbors[SOUTH] = &sides[NY];
	sides[PZ].neighbors[NORTH] = &sides[PY];

	sides[NY].neighbors[NORTH] = &sides[PX];
	sides[NY].neighbors[WEST] = &sides[NZ];
	sides[NY].neighbors[EAST] = &sides[PZ];
	sides[NY].neighbors[SOUTH] = &sides[NX];

	sides[NZ].neighbors[EAST] = &sides[NX];
	sides[NZ].neighbors[NORTH] = &sides[PY];
	sides[NZ].neighbors[SOUTH] = &sides[NY];
	sides[NZ].neighbors[WEST] = &sides[PX];

	sides[PX].planetside = PX;
	sides[PY].planetside = PY;
	sides[PZ].planetside = PZ;
	sides[NX].planetside = NX;
	sides[NY].planetside = NY;
	sides[NZ].planetside = NZ;

	// Render sides
	render_sides[PX].neighbors[NORTH] = &render_sides[PY];
	render_sides[PX].neighbors[EAST] = &render_sides[NZ];
	render_sides[PX].neighbors[SOUTH] = &render_sides[NY];
	render_sides[PX].neighbors[WEST] = &render_sides[PZ];

	render_sides[NX].neighbors[NORTH] = &render_sides[NY];
	render_sides[NX].neighbors[EAST] = &render_sides[PZ];
	render_sides[NX].neighbors[SOUTH] = &render_sides[PY];
	render_sides[NX].neighbors[WEST] = &render_sides[NZ];

	render_sides[PY].neighbors[NORTH] = &render_sides[NX];
	render_sides[PY].neighbors[SOUTH] = &render_sides[PX];
	render_sides[PY].neighbors[EAST] = &render_sides[PZ];
	render_sides[PY].neighbors[WEST] = &render_sides[NZ];

	render_sides[PZ].neighbors[EAST] = &render_sides[PX];
	render_sides[PZ].neighbors[WEST] = &render_sides[NX];
	render_sides[PZ].neighbors[SOUTH] = &render_sides[NY];
	render_sides[PZ].neighbors[NORTH] = &render_sides[PY];

	render_sides[NY].neighbors[NORTH] = &render_sides[PX];
	render_sides[NY].neighbors[WEST] = &render_sides[NZ];
	render_sides[NY].neighbors[EAST] = &render_sides[PZ];
	render_sides[NY].neighbors[SOUTH] = &render_sides[NX];

	render_sides[NZ].neighbors[EAST] = &render_sides[NX];
	render_sides[NZ].neighbors[NORTH] = &render_sides[PY];
	render_sides[NZ].neighbors[SOUTH] = &render_sides[NY];
	render_sides[NZ].neighbors[WEST] = &render_sides[PX];

	render_sides[PX].planetside = PX;
	render_sides[PY].planetside = PY;
	render_sides[PZ].planetside = PZ;
	render_sides[NX].planetside = NX;
	render_sides[NY].planetside = NY;
	render_sides[NZ].planetside = NZ;
}


QuadTreePlanet::~QuadTreePlanet()
{
}



void QuadTreePlanet::do_imgui(PlanetTileServer* server)
{
	ImGui::Text("X (P/N)");

	const int SIZE = 128;

	ImGui::BeginChild("PX", ImVec2(SIZE, SIZE));
	sides[PX].draw_gui(SIZE - 1, server);
	ImGui::EndChild();
	ImGui::SameLine();
	ImGui::BeginChild("NX", ImVec2(SIZE, SIZE));
	sides[NX].draw_gui(SIZE - 1, server);
	ImGui::EndChild();

	ImGui::SameLine();

	ImGui::BeginChild("RPX", ImVec2(SIZE, SIZE));
	render_sides[PX].draw_gui(SIZE - 1, server);
	ImGui::EndChild();
	ImGui::SameLine();
	ImGui::BeginChild("RNX", ImVec2(SIZE, SIZE));
	render_sides[NX].draw_gui(SIZE - 1, server);
	ImGui::EndChild();

	ImGui::Text("Y (P/N)");

	ImGui::BeginChild("PY", ImVec2(SIZE, SIZE));
	sides[PY].draw_gui(SIZE - 1, server);
	ImGui::EndChild();
	ImGui::SameLine();
	ImGui::BeginChild("NY", ImVec2(SIZE, SIZE));
	sides[NY].draw_gui(SIZE - 1, server);
	ImGui::EndChild();

	ImGui::SameLine();

	ImGui::BeginChild("RPY", ImVec2(SIZE, SIZE));
	render_sides[PY].draw_gui(SIZE - 1, server);
	ImGui::EndChild();
	ImGui::SameLine();
	ImGui::BeginChild("RNY", ImVec2(SIZE, SIZE));
	render_sides[NY].draw_gui(SIZE - 1, server);
	ImGui::EndChild();


	ImGui::Text("Z (P/N)");

	ImGui::BeginChild("PZ", ImVec2(SIZE, SIZE));
	sides[PZ].draw_gui(SIZE - 1, server);
	ImGui::EndChild();
	ImGui::SameLine();
	ImGui::BeginChild("NZ", ImVec2(SIZE, SIZE));
	sides[NZ].draw_gui(SIZE - 1, server);
	ImGui::EndChild();

	ImGui::SameLine();

	ImGui::BeginChild("RPZ", ImVec2(SIZE, SIZE));
	render_sides[PZ].draw_gui(SIZE - 1, server);
	ImGui::EndChild();
	ImGui::SameLine();
	ImGui::BeginChild("RNZ", ImVec2(SIZE, SIZE));
	render_sides[NZ].draw_gui(SIZE - 1, server);
	ImGui::EndChild();

	ImGui::Text("Wanted Depth: %i", (int)wanted_depth);

}

