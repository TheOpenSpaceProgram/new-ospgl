#pragma once
#include <glm/glm.hpp>
#pragma warning(push, 0)
#include <LinearMath/btVector3.h>
#include <glm/gtx/quaternion.hpp>
#pragma warning(pop)

// TODO: Maybe  pass by reference is more perfomant?

inline glm::dvec3 to_dvec3(const btVector3 data)
{
	return glm::dvec3((double)data.x(), (double)data.y(), (double)data.z());
} 

inline glm::vec3 to_vec3(const btVector3 data)
{
	return glm::vec3((float)data.x(), (float)data.y(), (float)data.z());
}

inline glm::dquat to_dquat(const btQuaternion data)
{
	return glm::dquat((double)data.w(), (double)data.x(), (double)data.y(), (double)data.z());
}

inline glm::dmat4 to_dmat4(const btTransform data)
{
	glm::dvec3 off = to_dvec3(data.getOrigin());
	glm::dquat rot = to_dquat(data.getRotation());

	glm::dmat4 mat = glm::translate(glm::dmat4(1.0), off);
	mat = glm::toMat4(rot) * mat;

	return mat;
}

template<typename T>
inline btVector3 to_btVector3(glm::tvec3<T> d)
{
	return btVector3((btScalar)d.x, (btScalar)d.y, (btScalar)d.z);
}

template<typename T>
inline btQuaternion to_btQuaternion(glm::tquat<T> d)
{
	return btQuaternion((btScalar)d.x, (btScalar)d.y, (btScalar)d.z, (btScalar)d.w);
}