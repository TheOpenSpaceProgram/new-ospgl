#include "Entity.h"
#include "../Universe.h"
#include "entities/VehicleEntity.h"
#include "entities/BuildingEntity.h"


void Entity::setup(Universe* universe, int64_t uid)
{
	this->universe = universe;
	this->uid = uid;
	init();
}

Entity::~Entity()
{
}

Entity* Entity::load(std::string type, cpptoml::table& toml)
{
	Entity* n_ent = nullptr;

	if (type == "vehicle")
	{
		n_ent = new VehicleEntity(toml);
	}
	else if (type == "building")
	{
		n_ent = new BuildingEntity(toml);
	}
	else
	{
		logger->fatal("Unknown entity type '{}'", type);
	}

	return n_ent;
}
