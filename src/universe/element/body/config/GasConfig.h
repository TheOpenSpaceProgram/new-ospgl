#pragma once

struct GasConfig 
{

};


template<>
class GenericSerializer<GasConfig>
{
public:

	static void serialize(const GasConfig& what, cpptoml::table& target)
	{

	}

	static void deserialize(GasConfig& to, const cpptoml::table& from)
	{

	}
};