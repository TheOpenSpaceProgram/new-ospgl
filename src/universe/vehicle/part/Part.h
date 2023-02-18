#pragma once
#include "Piece.h"
#include "Machine.h"
#include <vector>
#include <assets/PartPrototype.h>

// The vessel has many pieces, out of which some are "root"
// Those root pieces point to a Part which stores data and behaviour
// BUT, machines may have an assigned piece. In the case of this piece going
// missing, the machine will be disabled. Useful in many cases
class Part
{
private:

	// We keep it for initizalition
	std::shared_ptr<cpptoml::table> our_table;

public:

	Vehicle* vehicle;

	// IDs are unique in the whole universe
	int64_t id;
	// -1 means no group (default group)
	int64_t group_id;

	AssetHandle<PartPrototype> part_proto;

	std::unordered_map<std::string, Machine*> machines;

	// Attached machines are not in the part prototype, but attached by other means
	// They are serialized by array index
	// Used for plumbing machines
	std::vector<Machine*> attached_machines;

	// "p_root" is always the root, and is always present
	// When a piece goes missing, it turns to null, but the map entry stays
	std::unordered_map<std::string, Piece*> pieces;

	void pre_update(double dt);
	void update(double dt);
	void editor_update(double dt);

	void physics_update(double dt);

	void init(sol::state* st, Vehicle* in_vehicle);

	// Throws an error on the case of an invalid name,
	// return null if piece exists but is not present
	Piece* get_piece(const std::string& name) const;

	Machine* get_machine(const std::string& id) const;

	// Creates all pieces neccesary
	// Used by the editor as the parts are created from scratch
	// and not loaded from a file which already contains the pieces
	// All pieces assume p_root is untransformed for positioning
	// Must be called in packed mode as we set packed_tform
	std::vector<Piece*> create_pieces();

	// Assigns special ids to attached_machines which start with _attached
	std::unordered_map<std::string, Machine*> get_all_machines() const;

	// We duplicate the asset handle
	// our_table must contain any extra arguments to machines
	Part(AssetHandle<PartPrototype>& part_proto, std::shared_ptr<cpptoml::table> our_table);
	~Part();
};

