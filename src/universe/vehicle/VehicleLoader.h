#pragma once
#include "Vehicle.h"
#include <util/SerializeUtil.h>
#include <util/serializers/glm.h>
#include <physics/glm/BulletGlmCompat.h>


// This is used internally by Vehicle::deserialize as it's cleaner this way
// so it's not a single huge function
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


	void load_metadata(const cpptoml::table& root);
	void obtain_parts(const cpptoml::table& root);
	void obtain_pieces(const cpptoml::table& root);
	void copy_pieces(const cpptoml::table& root);
	void obtain_wires(const cpptoml::table& root);

	Piece* load_piece(const cpptoml::table& piece);

public:

	VehicleLoader(const cpptoml::table& root, Vehicle& to);
};
