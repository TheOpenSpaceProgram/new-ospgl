#pragma once
#include <glm/glm.hpp>
#pragma warning(push, 0)
#include <LinearMath/btQuaternion.h>
#include <LinearMath/btTransform.h>
#include <LinearMath/btVector3.h>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
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
	btScalar matrix[16];

	data.getOpenGLMatrix(&matrix[0]);

	return glm::dmat4(
		matrix[0], matrix[1], matrix[2], matrix[3],
		matrix[4], matrix[5], matrix[6], matrix[7],
		matrix[8], matrix[9], matrix[10], matrix[11],
		matrix[12], matrix[13], matrix[14], matrix[15]);
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

inline btTransform to_btTransform(glm::dmat4 mat)
{
	btTransform out = btTransform::getIdentity();
	out.setFromOpenGLMatrix(glm::value_ptr(mat));
	return out;
}

