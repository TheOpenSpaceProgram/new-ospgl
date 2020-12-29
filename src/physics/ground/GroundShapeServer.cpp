#include "GroundShapeServer.h"




btVector3* GroundShapeServer::query(QuadTreeNode* node, double time)
{
	PlanetTilePath path = PlanetTilePath(node->get_path(), node->planetside);

	if (cache.find(path) != cache.end())
	{
		return &cache[path]->verts[0];
	}
	else
	{
		// We must generate a new cache entry
		TileAndTriangles* n_tile = new TileAndTriangles(path, time, this);
		cache[path] = n_tile;

		return &n_tile->verts[0];
	}
}

GroundShapeServer::GroundShapeServer(SystemElement* body)
{
	this->body = body;

	bool wrote_error = false;

	std::string script = AssetManager::load_string_raw(body->config.surface.script_path);

	PlanetTile::prepare_lua(lua);
	LuaUtil::safe_lua(lua, script, wrote_error, body->config.surface.script_path);

	PlanetTile::generate_physics_index_array(indices);
}


GroundShapeServer::~GroundShapeServer()
{
}

GroundShapeServer::TileAndTriangles::TileAndTriangles(PlanetTilePath npath, double time, GroundShapeServer* server) 
	: path(npath)
{
	//double growth = -2.1500;
	double growth = -2.5; // A little excessive so vehicles "sink" a little and dont float
	double planet_radius = server->body->config.radius + growth;

	PlanetTile::generate_physics(npath, server->body->config.radius, server->lua, &server->work_array);

	glm::dmat4 model = glm::dmat4(1.0);
	model = glm::scale(model, glm::dvec3(planet_radius));
	model = model * path.get_model_spheric_matrix();

	for (size_t i = 0; i < server->indices.size(); i++)
	{
		glm::dvec3 v = server->work_array[server->indices[i]].pos;
		// Transform to real position relative to planet
		v = model * glm::dvec4(v, 1.0);

		verts[i] = to_btVector3(v);
	}
}
