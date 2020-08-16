#pragma once

#pragma warning(push, 0)
#include <btBulletCollisionCommon.h>
#pragma warning(pop)

#include <universe/element/body/PlanetaryBody.h>
#include "../glm/BulletGlmCompat.h"
#include <util/DebugDrawer.h>
#include "GroundShapeServer.h"
class GroundShape : public btConcaveShape
{
private:

	btVector3 m_localScaling;
	PlanetaryBody* body;
	GroundShapeServer* server;


public:
	
	double cur_system_t;

	glm::dvec3 cur_offset;

	virtual void getAabb(const btTransform& t, btVector3& aabbMin, btVector3& aabbMax) const;
	virtual void processAllTriangles(btTriangleCallback * callback, const btVector3& aabbMin, const btVector3& aabbMax) const;

	void calculateLocalInertia(btScalar, btVector3& inertia) const
	{
		inertia.setValue(btScalar(0.), btScalar(0.), btScalar(0.));
	}

	void setLocalScaling(const btVector3& scaling)
	{
		m_localScaling = scaling;
	}

	const btVector3& getLocalScaling() const
	{
		return m_localScaling;
	}

	virtual const char*	getName() const { return "PROCTERRAIN"; }

	GroundShape(PlanetaryBody* body);
	~GroundShape();
};

