#include "VehicleLoader.h"

void VehicleLoader::load_metadata(const cpptoml::table& root)
{
	// TODO: Description and package check
	vpart_id = *root.get_qualified_as<int64_t>("part_id");
	vpiece_id = *root.get_qualified_as<int64_t>("piece_id");

}

void VehicleLoader::obtain_parts(const cpptoml::table& root)
{
	auto parts = root.get_table_array_qualified("part");	
	if(!parts)
	{
		return;
	}

	for(auto part : *parts)
	{
		std::string proto_path = *part->get_qualified_as<std::string>("proto");
		AssetHandle<PartPrototype> part_proto = AssetHandle<PartPrototype>(proto_path);
		Part* n_part = new Part(part_proto, part);

		n_part->id = *part->get_qualified_as<int64_t>("id");
		logger->check(n_part->id <= vpart_id, "Malformed vehicle, part ID too big ({}/{})", 
				n_part->id, vpart_id);

		n_vehicle->parts.push_back(n_part);
		parts_by_id[n_part->id] = n_part;
	}


}

Piece* VehicleLoader::load_piece(const cpptoml::table& piece)
{
	int64_t part_id = *piece.get_qualified_as<int64_t>("part");
	std::string node = *piece.get_qualified_as<std::string>("node");
	bool is_root_of_part = node == "p_root";

	int64_t piece_id = *piece.get_qualified_as<int64_t>("id");
	
	Piece* n_piece = new Piece(parts_by_id[part_id], node);
	n_piece->id = piece_id;

	// Add ourselves to the part
	logger->check(parts_by_id.find(part_id) != parts_by_id.end(), "Invalid part ID ({})", part_id);
	Part* part = parts_by_id[part_id];
	logger->check(part->pieces.find(node) == part->pieces.end(), "Duplicate piece of part");
	part->pieces[node] = n_piece;
	n_piece->part = part;

	// Load transform
	auto transform = piece.get_array("transform");
	glm::dmat4 tform = deserialize_matrix(*transform);
	n_piece->packed_tform = to_btTransform(tform);

	// Pre-load links (We cannot load them just yet)	
	auto link = piece.get_table_qualified("link");
	links_toml[n_piece] = link;
	
	pieces_by_id[piece_id] = n_piece;

	return n_piece;
}

void VehicleLoader::obtain_pieces(const cpptoml::table& root)
{
	auto pieces = root.get_table_array_qualified("piece");
	root_piece = nullptr;

	if(!pieces)
	{
		return;
	}

	for(auto piece : *pieces)
	{
		Piece* n_piece = load_piece(*piece);

		auto root_entry = piece->get_qualified_as<bool>("root");
		if(root_entry && *root_entry == true)
		{
			logger->check(root_piece == nullptr, "Multiple root pieces, that's invalid");
			root_piece = n_piece;
		}
		else
		{
			all_pieces.push_back(n_piece);
		}


		n_piece->in_vehicle = n_vehicle;
	}

}

void VehicleLoader::copy_pieces(const cpptoml::table& root)
{
	if(root_piece == nullptr)
	{
		n_vehicle->root = nullptr;
		return;
	}

	n_vehicle->all_pieces.push_back(root_piece);
	n_vehicle->root = root_piece;
	for(Piece* p : all_pieces)
	{
		n_vehicle->all_pieces.push_back(p);

		auto link = links_toml[p];
		if(link)
		{
			int64_t to = *link->get_qualified_as<int64_t>("to");
			logger->check(pieces_by_id.find(to) != pieces_by_id.end(), "Link to a non-existant piece {}", to);
			Piece* to_p = pieces_by_id[to];
			
			p->attached_to = to_p;
			
			p->to_attachment = link->get_as<std::string>("to_attachment").value_or("");
			p->from_attachment = link->get_as<std::string>("from_attachment").value_or(""); 
			
			bool welded = link->get_qualified_as<bool>("welded").value_or(false);
			p->welded = welded;

			p->editor_dettachable = link->get_qualified_as<bool>("editor_dettachable").value_or(true);

			std::string link_type = link->get_qualified_as<std::string>("type").value_or("none");
			if(link_type != "none")
			{
				// Load the physical link
				p->link = std::make_unique<Link>(osp->assets->load_script(link_type).first);

				glm::dvec3 link_from, link_to;
				deserialize(p->link_from, *link->get_table_qualified("pfrom"));	
				deserialize(p->link_to, *link->get_table_qualified("pto"));
				deserialize(p->link_rot, *link->get_table_qualified("rot"));
			}
		}
	}
}

void VehicleLoader::obtain_wires(const cpptoml::table& root)
{
	auto wires = root.get_table_array_qualified("wire");
	if(!wires)
	{
		return;
	}

	for(auto wire_toml : *wires)
	{
		int from = *wire_toml->get_as<int>("from");
		int to = *wire_toml->get_as<int>("to");
		std::string fmachine = *wire_toml->get_as<std::string>("fmachine");
		std::string tmachine = *wire_toml->get_as<std::string>("tmachine");

		Part* fromp = parts_by_id[from];
		Part* top = parts_by_id[to];
		Machine* fromm = fromp->get_machine(fmachine);
		Machine* tom = top->get_machine(tmachine);

		// Check if it already exists bidirectionally to emit a warning
		// TODO: Move this code to a function as it may be reused
		auto from_it = n_vehicle->wires.equal_range(fromm);
		bool found_from_to = false;
		for(auto from_subit = from_it.first; from_subit != from_it.second; from_subit++)
		{
			if(from_subit->second == tom)
			{
				found_from_to = true;
				break;
			}
		}

		auto to_it = n_vehicle->wires.equal_range(tom);
		bool found_to_from = false;
		for(auto to_subit = to_it.first; to_subit != to_it.second; to_subit++)
		{
			if(to_subit->second == fromm)
			{
				found_to_from = true;
				break;
			}
		}

		if(!found_from_to)
		{
			n_vehicle->wires.insert(std::make_pair(fromm, tom));
		}

		if(!found_to_from)
		{
			n_vehicle->wires.insert(std::make_pair(tom, fromm));
		}

		if(found_from_to || found_to_from)
		{
			logger->warn("Found a duplicate wire (from: {} fmachine: {} -> to: {} tmachine: {}), it was ignored",
				from, fmachine, to,  tmachine);
		}
	}
}

VehicleLoader::VehicleLoader(const cpptoml::table& root, Vehicle& to)
{
	n_vehicle = &to;

	load_metadata(root);
	obtain_parts(root);
	obtain_pieces(root);
	copy_pieces(root);
	obtain_wires(root);

	n_vehicle->id_to_part = parts_by_id;
	n_vehicle->id_to_piece = pieces_by_id;

	if(n_vehicle->root == nullptr)
	{
		logger->fatal("Loaded an empty vehicle!");
	}

	// Initialize the vehicle's physical state to packed and save default values
	WorldState n_state;
	n_vehicle->packed_veh.set_world_state(n_state);
	n_vehicle->packed = true;

	// Calculate the packed COM
	n_vehicle->packed_veh.calculate_com();

	n_vehicle->update_attachments();

}

VehicleSaver::VehicleSaver(cpptoml::table &target, const Vehicle &what)
{
	logger->check(what.is_packed(), "Cannot serialize a unpacked vehicle");
	assign_ids(target, what);
	write_parts(target, what);
	write_pieces(target, what);
	write_wires(target, what);
}

void VehicleSaver::assign_ids(cpptoml::table& target, const Vehicle& what)
{
	int64_t piece_id = 0;
	int64_t part_id = 0;
	for(Piece* p : what.all_pieces)
	{
		piece_id++;
		piece_to_id[p] = piece_id;
	}

	for(Part* p : what.parts)
	{
		part_id++;
		part_to_id[p] = part_id;
	}

	// These are used as a sanity check
	target.insert("piece_id", piece_id);
	target.insert("part_id", part_id);
}

void VehicleSaver::write_parts(cpptoml::table &target, const Vehicle &what)
{
	auto part_array = cpptoml::make_table_array();
	for(auto pair : part_to_id)
	{
		auto table = cpptoml::make_table();

		table->insert("id", pair.second);
		table->insert("proto", pair.first->part_proto.pkg + ":" + pair.first->part_proto.name);

		for(auto m_pair : pair.first->machines)
		{
			auto m_table = cpptoml::make_table();
			// TODO: Allow machines to write wathever they want to the toml from lua

			// Plumbing metadata
			if(m_pair.second->plumbing.has_lua_plumbing())
			{
				m_table->insert("plumbing_rot", m_pair.second->plumbing.editor_rotation);
				serialize_to_table(m_pair.second->plumbing.editor_position, *m_table, "plumbing_pos");
			}

			if(!table->empty())
			{
				table->insert(m_pair.first, m_table);
			}
		}

		part_array->push_back(table);
	}
	target.insert("part", part_array);
}

void VehicleSaver::write_pieces(cpptoml::table &target, const Vehicle &what)
{
	auto piece_array = cpptoml::make_table_array();
	for(auto pair : piece_to_id)
	{
		auto table = cpptoml::make_table();

		table->insert("id", pair.second);
		if(pair.first->part != nullptr)
		{
			table->insert("part", part_to_id[pair.first->part]);
		}
		else
		{
			// TODO: We need to insert the prototype anyway to load the piece properties
			// ? Insert the asset path of the prototype
		}

		if(what.root == pair.first)
		{
			table->insert("root", true);
		}

		table->insert("node", pair.first->piece_prototype->name);
		auto matrix = serialize_matrix(to_dmat4(pair.first->packed_tform));
		table->insert("transform", matrix);


		if(pair.first->attached_to != nullptr)
		{
			// Serialize the link
			auto link = cpptoml::make_table();

			link->insert("to", piece_to_id[pair.first->attached_to]);
			if(pair.first->welded)
			{
				link->insert("welded", true);
			}

			// This is kind of an "editor-metadata" but it's very important
			if(!pair.first->editor_dettachable)
			{
				link->insert("editor_dettachable", false);
			}

			// TODO: Insert links, link orientation, etc...

			if(pair.first->from_attachment != "")
			{
				link->insert("from_attachment", pair.first->from_attachment);
			}

			if(pair.first->to_attachment != "")
			{
				link->insert("to_attachment", pair.first->to_attachment);
			}

			table->insert("link", link);
		}

		piece_array->push_back(table);
	}
	target.insert("piece", piece_array);
}

void VehicleSaver::write_wires(cpptoml::table &target, const Vehicle &what)
{
	auto wire_array = cpptoml::make_table_array();

	std::set<std::pair<Machine*, Machine*>> seen_pairs;

	for(auto pair : what.wires)
	{
		auto rpair = std::make_pair(pair.second, pair.first);
		if(seen_pairs.find(pair) == seen_pairs.end() &&
		   seen_pairs.find(rpair) == seen_pairs.end())
		{
			seen_pairs.insert(pair);

			auto wire = cpptoml::make_table();

			std::string fmachine = "";
			std::string tmachine = "";
			for(auto m : pair.first->in_part->machines)
			{
				if(m.second == pair.first)
				{
					fmachine = m.first;
				}
			}
			for(auto m : pair.second->in_part->machines)
			{
				if(m.second == pair.second)
				{
					tmachine = m.first;
				}
			}

			logger->check(fmachine != "" && tmachine != "", "Invalid machines");

			wire->insert("from", part_to_id[pair.first->in_part]);
			wire->insert("fmachine", fmachine);
			wire->insert("to", part_to_id[pair.second->in_part]);
			wire->insert("tmachine", tmachine);

			wire_array->push_back(wire);
		}
	}
	target.insert("wire", wire_array);
}
