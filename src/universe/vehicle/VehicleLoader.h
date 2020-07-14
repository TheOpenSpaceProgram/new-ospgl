#pragma once
#include "Vehicle.h"
#include <util/SerializeUtil.h>
#include <util/serializers/glm.h>
#include <physics/glm/BulletGlmCompat.h>

// We don't use traditional serialization because this is a huge task
class VehicleLoader
{
private:

	int64_t vpart_id;
	int64_t vpiece_id;

	Vehicle* n_vehicle;
	std::unordered_map<int64_t, Part*> parts_by_id;
	std::unordered_map<int64_t, Piece*> pieces_by_id;
	std::unordered_map<Piece*, std::shared_ptr<cpptoml::table>> links_toml;
	std::vector<Piece*> all_pieces;
	Piece* root_piece;


	void load_metadata(cpptoml::table& root);
	void obtain_parts(cpptoml::table& root);
	void obtain_pieces(cpptoml::table& root);
	void copy_pieces(cpptoml::table& root);
	void obtain_wires(cpptoml::table& root);

	Piece* load_piece(cpptoml::table& piece);

public:

	Vehicle* get_vehicle();

	VehicleLoader(cpptoml::table& root);
};
