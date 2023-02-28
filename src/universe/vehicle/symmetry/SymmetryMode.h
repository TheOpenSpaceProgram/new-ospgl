#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include "../part/Part.h"

class Vehicle;

// A symmetric distribution handles all symmetric parts in a group
// Behaviour is handled in lua
class SymmetryMode
{
private:
	// Do not use keys starting with "__" as these are used by the engine
	std::shared_ptr<cpptoml::table> save_toml;

public:
	// Only root pieces are included here. Set on creation.
	// To obtain children, use vehicle functions
	// TODO: If this causes lags, store child pieces too and update the array appropiately!
	std::vector<Piece*> symmetry_roots;

	// Called when any of the mirrored pieces is modified
	void on_dirty(Piece* piece);

	// Calld when any of the mirrored, child pieces, is disconnected,
	// so that mirrored versions are properly deleted
	void on_disconnect(Piece* piece);

	SymmetryMode
};

