#pragma once
#include "Piece.h"
#include "Machine.h"
#include <vector>
#include <assets/PartPrototype.h>

// The vessel has many pieces, out of which some are "root"
// Those root pieces point to a Part which stores data and behaviour
class Part
{
private:

	// We keep it for initizalition
	std::shared_ptr<cpptoml::table> our_table;

public:

	Vehicle* vehicle;

	int64_t id;

	AssetHandle<PartPrototype> part_proto;

	std::unordered_map<std::string, Machine*> machines;

	// "p_root" is always the root, and is always present
	// When a piece goes missing, it turns to null, but the map entry stays
	std::unordered_map<std::string, Piece*> pieces;

	void pre_update(double dt);
	void update(double dt);
	void editor_update(double dt);

	void init(sol::state* st, Vehicle* in_vehicle);

	// Throws an error on the case of an invalid name,
	// return null if piece exists but is not present
	Piece* get_piece(const std::string& name);

	Machine* get_machine(const std::string& id);

	// Creates all pieces neccesary
	// Used by the editor as the parts are created from scratch
	// and not loaded from a file which already contains the pieces
	// All pieces assume p_root is untransformed for positioning
	// Must be called in packed mode as we set packed_tform
	std::vector<Piece*> create_pieces();

	// We duplicate the asset handle
	// our_table must contain any extra arguments to machines
	Part(AssetHandle<PartPrototype>& part_proto, std::shared_ptr<cpptoml::table> our_table);
	~Part();
};

