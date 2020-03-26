#include "VehicleLoader.h"


Vehicle* VehicleLoader::load_vehicle(cpptoml::table& root)
{
	Vehicle* n_vehicle = new Vehicle();

	// We are already inside vehicle_prototype		

	// TODO: Description and package check

	n_vehicle->part_id = *root.get_qualified_as<int64_t>("part_id");
	n_vehicle->piece_id = *root.get_qualified_as<int64_t>("piece_id");
	
	// Obtain parts
	auto parts = root.get_table_array_qualified("parts");	
	std::unordered_map<int64_t, Part*> parts_by_id;

	for(auto part : *parts)
	{
		std::string proto_path = *part->get_qualified_as<std::string>("proto");
		AssetHandle<PartPrototype> part_proto = AssetHandle<PartPrototype>(proto_path);
		Part* n_part = new Part(part_proto);

		n_part->id = *part->get_qualified_as<int64_t>("id");
		logger->check_important(n_part->id <= n_vehicle->part_id, "Malformed vehicle, part ID too big");

		parts_by_id[n_part->id] = n_part;
	}

	// Obtain pieces into our array, later we copy
	auto pieces = root.get_table_array_qualified("pieces");
	std::vector<Piece*> all_pieces;
	Piece* root_piece = nullptr;

	for(auto piece : *pieces)
	{
		int64_t part_id = *piece->get_qualified_as<int64_t>("part");
		std::string node = *piece->get_qualified_as<std::string>("node");
		bool is_root_of_part = node == "p_root";

		Piece* n_piece = new Piece(parts_by_id[part_id], node);
		
		// Load transform
		auto transform = piece->get_table_qualified("transform");
		auto pos = transform->get_table_qualified("pos");
		auto rot = transform->get_table_qualified("rot");
		glm::dvec3 pos_d; deserialize(pos_d, *pos);
		glm::dvec4 rot_d; deserialize(rot_d, *rot);
		n_piece->packed_tform.setIdentity();
		n_piece->packed_tform.setOrigin(to_btVector3(pos_d));
		n_piece->packed_tform.setRotation(to_btQuaternion(glm::dquat(rot_d.w, rot_d.x, rot_d.y, rot_d.z)));
		// Pre-load links (We cannot load them just yet)

		auto root_entry = piece->get_qualified_as<bool>("root");
		if(root_entry && *root_entry == true)
		{
			logger->check_important(root_piece == nullptr, "Multiple root pieces, that's invalid");
			root_piece = n_piece;
		}

		all_pieces.push_back(n_piece);
	}


	return n_vehicle;
}
