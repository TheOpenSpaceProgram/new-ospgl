#pragma once

template<>
class GenericSerializer<PlanetarySystem>
{
public:

	

	static void serialize(const PlanetarySystem& what, cpptoml::table& target)
	{

	}

	static void deserialize(PlanetarySystem& to, const cpptoml::table& from)
	{
		to.load(from);
	}
};