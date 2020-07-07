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

		Machine* n_machine = new Machine(config_toml, cur_pkg);
		machines[id] = n_machine;
	}
}

void Part::pre_update(double dt)
{
	for(auto& machine_pair : machines)
	{
		machine_pair.second->pre_update(dt);
	}
}

void Part::update(double dt)
{
	for(auto& machine_pair : machines)
	{
		machine_pair.second->update(dt);
	}
}

void Part::editor_update(double dt)
{
	for(auto& machine_pair : machines)
	{
		machine_pair.second->editor_update(dt);
	}

}


void Part::init(Universe* in_universe, Vehicle* in_vehicle)
{
	this->vehicle = in_vehicle;

	for(auto& machine_pair : machines)
	{
		machine_pair.second->init(this, in_universe);
		machine_pair.second->define_ports();
	}

}

Piece* Part::get_piece(const std::string& name)
{
	auto it = pieces.find(name);
	logger->check(it != pieces.end(), "Tried to get a piece which does not exist on the part ({})", name);
	return it->second;
}

Machine* Part::get_machine(const std::string& id)
{
	auto it = machines.find(id);
	logger->check(it != machines.end(), "Invalid machine ID '{}'", id);
	return it->second;
}


std::vector<Piece*> Part::create_pieces() 
{
	std::vector<Piece*> n_pieces;

	for(auto pair : part_proto->pieces)
	{
		Piece* p = new Piece(this, pair.first);
		// TODO: Check this
		p->packed_tform = p->piece_prototype->collider_offset;
		p->in_vehicle = vehicle;
		pieces[pair.first] = p;
		n_pieces.push_back(p);
	}			

	// After everything is loaded, parts are responsible for creating
	// attachments between the different pieces
	// TODO: Create that functionality

	// TODO: Afterwards check that all pieces are attached so there is a single root

	return n_pieces;
}

Part::~Part()
{
	for(auto& machine_pair : machines)
	{
		delete machine_pair.second;
	}
}
