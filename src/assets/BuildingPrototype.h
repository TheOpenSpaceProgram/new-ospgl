#pragma once
#include "Model.h"

// A building can have many functionalities, one collider
// and a model (Use compound colliders for advanced shapes)
class BuildingPrototype
{
public:

	GPUModelPointer model;
	btCollisionShape* collider;
};


BuildingPrototype* load_building_prototype(const std::string& path, const std::string& name, const std::string& pkg, const cpptoml::table& cfg);
