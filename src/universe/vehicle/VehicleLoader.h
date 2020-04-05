#pragma once
#include "Vehicle.h"
#include <util/SerializeUtil.h>
#include <util/serializers/glm.h>
#include <physics/glm/BulletGlmCompat.h>

class VehicleLoader
{
public:

	static Vehicle* load_vehicle(cpptoml::table& root);

};
