#pragma once
#include "Piece.h"
#include "Machine.h"
#include <vector>
#include "../../assets/PartPrototype.h"

// The vessel has many pieces, out of which some are "root"
// Those root pieces point to a Part which stores data and behaviour
class Part
{
public:

	//AssetHandle<PartPrototype> proto;

	std::vector<Machine> machines;

	// "p_root" is always the root, and is always present
	// When a piece goes missing, it turns to null, but the map entry stays
	std::unordered_map<std::string, Piece*> pieces;

	Part();
	~Part();
};

