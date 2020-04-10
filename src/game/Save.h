#pragma once
#include <universe/Universe.h>
#include <OSP.h>
#include <util/SerializeUtil.h>
#include "universe/Date.h"

// Loads, runs, and stores a game state
class Save
{
public:

	OSP* osp;
	Universe universe;

	// Can be nullptr, will receive all input commands
	Entity* under_control;

	void load(const cpptoml::table& from);
	void write(cpptoml::table& target) const;

	Save(OSP* osp);
};


template<>
class GenericSerializer<Save>
{
public:

	static void serialize(const Save& what, cpptoml::table& target)
	{
		what.write(target);
	}

	static void deserialize(Save& to, const cpptoml::table& from)
	{
		to.load(from);
	}
};
