#pragma once
#include <LinearMath/btIDebugDraw.h>
#include "../../util/DebugDrawer.h"
#include "../glm/BulletGlmCompat.h"

class BulletDebugDrawer : public btIDebugDraw
{
private:

	int m_debugMode;

public:

	virtual void drawLine(const btVector3& from, const btVector3& to, const btVector3& color)
	{
		debug_drawer->add_line(to_dvec3(from), to_dvec3(to), to_vec3(color));
	}

	virtual void drawLine(const btVector3& from, const btVector3& to, const btVector3& fromColor, const btVector3& toColor)
	{
		debug_drawer->add_line(to_dvec3(from), to_dvec3(to), to_vec3(fromColor), to_vec3(toColor));
	}

	virtual void drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color)
	{
		debug_drawer->add_point(to_dvec3(PointOnB), to_vec3(color));
		debug_drawer->add_line(to_dvec3(PointOnB), to_dvec3(PointOnB) + to_dvec3(normalOnB) * distance, to_vec3(color));
	}

	virtual void reportErrorWarning(const char* warningString)
	{
		logger->warn("[Bullet] {}", warningString);
	}

	virtual void draw3dText(const btVector3& location, const char* textString)
	{
		// TODO
	}

	virtual void setDebugMode(int debugMode)
	{
		m_debugMode = debugMode;
	}

	virtual int getDebugMode() const { return m_debugMode; }

	BulletDebugDrawer() {}
	~BulletDebugDrawer() {}
};

