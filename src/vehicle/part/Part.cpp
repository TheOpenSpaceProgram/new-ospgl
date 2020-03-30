#include "Part.h"
#include <string>

Part::Part(AssetHandle<PartPrototype>& part_proto, cpptoml::table& our_table)
{
	this->part_proto = part_proto.duplicate();

	// Load machines
	for(auto machine_toml : part_proto->machines)
	{
		std::string id = *machine_toml->get_as<std::string>("id");
		std::string script = *machine_toml->get_as<std::string>("script");

		auto config_toml = machine_toml;
		auto override_toml = our_table.get_table(id);
		if(override_toml)
		{
			SerializeUtil::override(*config_toml, *override_toml);
		}		

		std::string cur_pkg = part_proto.pkg;

		machines.emplace(std::piecewise_construct, std::make_tuple(id), std::make_tuple(config_toml, cur_pkg));
	}
}

void Part::update(double dt)
{
	for(auto& machine_pair : machines)
	{
		machine_pair.second.update(dt);
	}
}

void Part::init(Universe* in_universe, Vehicle* in_vehicle)
{
	this->vehicle = in_vehicle;

	for(auto& machine_pair : machines)
	{
		machine_pair.second.init(this, in_universe);
	}
}

Piece* Part::get_piece(const std::string& name)
{
	auto it = pieces.find(name);
	logger->check_important(it != pieces.end(), "Tried to get a piece which does not exist on the part");
	return it->second;
}

Part::~Part()
{
}
