#pragma once
#include "QuadTreeDefines.h"
#include "QuadTreeNode.h"
#include "../mesher/PlanetTilePath.h"
#include "../../util/MathUtil.h"

class PlanetTileServer;

// A QuadTreePlanet guarantees proper splitting
// of the tree, specially across side boundaries,
// and conversion between coordinate systems
// It also keeps another copy of the whole
// system used for rendering when small tiles
// are not yet generated
class QuadTreePlanet
{
private:

	glm::dvec2 wanted_pos;
	PlanetSide wanted_side;
	size_t wanted_depth;

	size_t current_depth;

	size_t previous_depth = 0;

	QuadTreeNode render_sides[6];

	uint64_t old_render_leafs_it;
	std::vector<PlanetTilePath> old_render_leafs;

public:

	// Used as an optimization so that get_leafs functions
	// store the previous result, if nothing changed
	uint64_t iteration;

	// TODO: Automatically set dirty flag
	bool dirty;

	void flatten();

	QuadTreeNode sides[6];
	
	std::vector<PlanetTilePath> get_all_render_leaf_paths(bool ignore_cache = false);

	// Recursively obtains all leafs from all sides, don't hold the
	// pointers for too long
	std::vector<QuadTreeNode*> get_all_leafs();

	// Converts the pointers to paths
	std::vector<PlanetTilePath> get_all_paths() const;


	// Gets the planet side a point is on from its normalized,
	// relative to the planet center, coordinates
	PlanetSide get_planet_side(glm::vec3 point_normalized) const;

	// Gets planet side offset given a point and the side it's contained in
	// (get it via get_planet_side)
	glm::dvec2 get_planet_side_offset(glm::vec3 point_normalized, PlanetSide side) const;

	void set_wanted_subdivide(glm::dvec2 offset, PlanetSide side, size_t depth);

	void do_imgui(PlanetTileServer& server);

	// Tries to update subdivision, if the server has finished building
	void update(PlanetTileServer& server);
	
	// Not used by the rendering code, but by physics
	void subdivide_to(glm::dvec2 offset, PlanetSide side, size_t depth);

	QuadTreePlanet();
	~QuadTreePlanet();
};

