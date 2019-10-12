#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include "../quadtree/QuadTreeDefines.h"
#include "../../util/defines.h"
#include "../../util/MathUtil.h"

struct PlanetTilePath
{
	std::vector<QuadTreeQuadrant> path;
	PlanetSide side;

	size_t get_depth() const;
	glm::dvec2 get_min() const;
	double get_size() const;

	glm::dvec3 get_tile_postrotation() const;
	glm::dvec3 get_tile_origin() const;
	glm::dvec3 get_tile_rotation() const;
	glm::dvec3 get_tile_translation(bool spheric = false) const;
	glm::dvec3 get_tile_scale() const;
	glm::dvec3 get_tile_postscale() const;

	glm::dmat4 get_model_matrix() const;
	glm::dmat4 get_model_spheric_matrix() const;



	PlanetTilePath(std::vector<QuadTreeQuadrant> path, PlanetSide side)
	{
		this->path = path;
		this->side = side;
	}
};

bool operator==(const PlanetTilePath& a, const PlanetTilePath& b);

struct PlanetTilePathHasher
{
	std::size_t operator()(const PlanetTilePath &t) const
	{
		std::size_t ret = 0;
		hash_combine(ret, t.side);
		for (size_t i = 0; i < t.path.size(); i++)
		{
			hash_combine(ret, t.path[i]);
		}

		return ret;
	}
};

// Used in ordered sets
struct PlanetTilePathLess
{
	// NOTE: We actually have the bigger tiles be the earliest 
	// on the set!
	bool operator() (const PlanetTilePath& a, const PlanetTilePath& b) const
	{
		return a.get_depth() < b.get_depth();
	}
};