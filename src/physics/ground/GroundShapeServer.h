#pragma once
#include "../../planet_mesher/quadtree/QuadTreeDefines.h"
#include "../../planet_mesher/quadtree/QuadTreeNode.h"
#include "../../planet_mesher/mesher/PlanetTile.h"
#include <glm/glm.hpp>

struct GroundTriangles
{

};

// Handles generation of the ground shape triangles,
// and, most importantly, caching of them using the
// quadtree coordinates.
// We use a time-out based system for "forgetting" about
// tiles as this may be useful in certain situations
// such as raycasting. Every request must tell the system
// how much to wait before dumping the tile. 
// We use physics dt, so lag should not make tiles instantly 
// disappear
class GroundShapeServer
{
public:

	void update(double pdt);

	
	GroundTriangles* query(QuadTreeNode* node);

	GroundShapeServer();
	~GroundShapeServer();
};

