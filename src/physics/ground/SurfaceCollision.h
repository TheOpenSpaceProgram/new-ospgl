#pragma once
#include <glm/glm.hpp>
#include <vector>


#pragma warning(push, 0)
#include <btBulletCollisionCommon.h>
#include <BulletCollision/Gimpact/btGImpactBvh.h>
#pragma warning(pop)

// Generates collision shapes as needed "under" 
// rigidbodies which may collide with the ground.
// We utilize a QuadTree, like usual, but we don't
// generate the whole planet, just leaf nodes of wanted depth
class SurfaceCollision
{
private:

public:



	SurfaceCollision();
	~SurfaceCollision();
};
