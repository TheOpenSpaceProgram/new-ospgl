#pragma once
#include "Model.h"
#include "Asset.h"


// A building can have many functionalities, one collider
// and a model (Use compound colliders for advanced shapes)
class BuildingPrototype : public Asset
{
public:

	GPUModelPointer model;
	btCollisionShape* collider;

	BuildingPrototype(std::shared_ptr<cpptoml::table> table, ASSET_INFO);
};


BuildingPrototype* load_building_prototype(ASSET_INFO, const cpptoml::table& cfg);
