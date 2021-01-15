#pragma once

// Gas giants also work as stars
// These don't have a proper surface and are "cloudly"
// They are semi-procedural, guided by the lua script
struct GasConfig 
{

};


template<>
class GenericSerializer<GasConfig>
{
public:

	NOT_SERIALIZABLE(GasConfig);

	static void deserialize(GasConfig& to, const cpptoml::table& from)
	{

	}
};