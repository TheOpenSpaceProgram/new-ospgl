#pragma once

#pragma warning(push, 0)
#include <btBulletCollisionCommon.h>
#pragma warning(pop)


class CollisionShape : public btConcaveShape
{
public:
	CollisionShape();
	~CollisionShape();
};

