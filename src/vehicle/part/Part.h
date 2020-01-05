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

	PartPrototype proto;

	std::vector<Machine> machines;

	// Index 0 is always the root subpart, which
	// MUST be present. Separated pieces simply disappear
	std::vector<Piece*> pieces;

	Part();
	~Part();
};

