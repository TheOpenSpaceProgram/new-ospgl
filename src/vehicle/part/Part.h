#pragma once
#include "Piece.h"

#include <vector>

// The vessel has many pieces, out of which some are "root"
// Those root pieces point to a Part which stores data and behaviour
class Part
{
public:

	// Index 0 is always the root subpart, which
	// MUST be present. Separated pieces simply disappear
	std::vector<Piece*> pieces;

	std::unordered_map<std::string, std::any> part_data;

	Part();
	~Part();
};

