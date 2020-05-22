#include "VehicleLoader.h"

Vehicle* VehicleLoader::get_vehicle()
{
	Vehicle* to_return = n_vehicle;
	n_vehicle = nullptr;
	// We give ownership to whoever takes the pointer, but don't 
	// use a smart pointer for code simplicity
	return to_return;
}

void VehicleLoader::load_metadata(cpptoml::table& root)
{
	// TODO: Description and package check
	n_vehicle->part_id = *root.get_qualified_as<int64_t>("part_id");
	n_vehicle->piece_id = *root.get_qualified_as<int64_t>("piece_id");

}

void VehicleLoader::obtain_parts(cpptoml::table& root)
{
	auto parts = root.get_table_array_qualified("part");	

	for(auto part : *parts)
	{
		std::string proto_path = *part->get_qualified_as<std::string>("proto");
		AssetHandle<PartPrototype> part_proto = AssetHandle<PartPrototype>(proto_path);
		Part* n_part = new Part(part_proto, *part);

		n_part->id = *part->get_qualified_as<int64_t>("id");
		logger->check(n_part->id <= n_vehicle->part_id, "Malformed vehicle, part ID too big ({}/{})", 
				n_part->id, n_vehicle->part_id);

		n_vehicle->parts.push_back(n_part);
		parts_by_id[n_part->id] = n_part;
	}


}

void VehicleLoader::obtain_pieces(cpptoml::table& root)
{
	auto pieces = root.get_table_array_qualified("piece");
	root_piece = nullptr;

	for(auto piece : *pieces)
	{
		int64_t part_id = *piece->get_qualified_as<int64_t>("part");
		std::string node = *piece->get_qualified_as<std::string>("node");
		bool is_root_of_part = node == "p_root";

		int64_t piece_id = *piece->get_qualified_as<int64_t>("id");
		
		Piece* n_piece = new Piece(parts_by_id[part_id], node);
		n_piece->id = piece_id;

		// Add ourselves to the part
		logger->check(parts_by_id.find(part_id) != parts_by_id.end(), "Invalid part ID ({})", part_id);
		Part* part = parts_by_id[part_id];
		logger->check(part->pieces.find(node) == part->pieces.end(), "Duplicate piece of part");
		part->pieces[node] = n_piece;
		n_piece->part = part;

		// Load transform
		auto pos = piece->get_table_qualified("pos");
		auto rot = piece->get_table_qualified("rot");
		glm::dvec3 pos_d; deserialize(pos_d, *pos);
		glm::dvec4 rot_d; deserialize(rot_d, *rot);
		n_piece->packed_tform.setIdentity();
		n_piece->packed_tform.setOrigin(to_btVector3(pos_d));
		n_piece->packed_tform.setRotation(to_btQuaternion(glm::dquat(rot_d.w, rot_d.x, rot_d.y, rot_d.z)));
		// Pre-load links (We cannot load them just yet)	
		auto link = piece->get_table_qualified("link");
		links_toml[n_piece] = link;

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

		pieces_by_id[piece_id] = n_piece;

		n_piece->in_vehicle = n_vehicle;
	}

}

void VehicleLoader::copy_pieces(cpptoml::table& root)
{
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
			
			bool welded = link->get_qualified_as<bool>("welded").value_or(false);
			p->welded = welded;

			std::string link_type = link->get_qualified_as<std::string>("type").value_or("none");
			if(link_type != "none")
			{
				// Load the physical link
				p->link = std::make_unique<Link>(assets->load_script(link_type));	

				glm::dvec3 link_from, link_to;
				deserialize(p->link_from, *link->get_table_qualified("pfrom"));	
				deserialize(p->link_to, *link->get_table_qualified("pto"));
				deserialize(p->link_rot, *link->get_table_qualified("rot"));
			}
		}
	}

}

VehicleLoader::VehicleLoader(cpptoml::table& root)
{
	n_vehicle = new Vehicle();

	load_metadata(root);
	obtain_parts(root);
	obtain_pieces(root);
	copy_pieces(root);

	// Wires are initialized after this by the vehicle
	// as machines need to start
	auto wires = root.get_table_array("wire");
	n_vehicle->wires_init = wires;

	n_vehicle->id_to_part = parts_by_id;
	n_vehicle->id_to_piece = pieces_by_id;

	// Initialize the vehicle's physical state to packed and save default values
	WorldState n_state;
	n_vehicle->packed_veh.set_world_state(n_state);
	n_vehicle->packed = true;

	// Calculate the packed COM
	n_vehicle->packed_veh.calculate_com();

	// Sort the vehicle so it's ready for physics
	n_vehicle->sort();
}
