#pragma once
#include <glm/glm.hpp>
#include <LinearMath/btVector3.h>

// TODO: Maybe  pass by reference is more perfomant?

glm::dvec3 to_dvec3(const btVector3 data)
{
	return glm::dvec3((double)data.x(), (double)data.y(), (double)data.z());
} 

glm::vec3 to_vec3(const btVector3 data)
{
	return glm::vec3((float)data.x(), (float)data.y(), (float)data.z());
}

template<typename T>
btVector3 to_btVector3(glm::tvec3<T> d)
{
	return btVector3((btScalar)d.x, (btScalar)d.y, (btScalar)d.z);
}
