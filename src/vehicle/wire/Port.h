#pragma once 
#include <stdint.h>
#include <string>
#include "../../util/Logger.h"

class Machine;



struct PortValue
{
	union
	{
		double as_number;
	};
};

class Port
{

public:

	enum Type
	{
		NUMBER
	};

	static std::string get_name(Port::Type type);
	static Port::Type get_type(const std::string& str);

	// This is lua's way to differentiate ports,
	// so they need to be unique. This is enforced
	// on port creation
	// (Port names must be unique inside of a part outputs
	// or inputs, you can have two ports named "x"" on the same
	// vehicle, and you can even have two ports named "x" on the same 
	// part, but they cannot be both input or output
	std::string name;

	bool is_output;

	Type type;
	Machine* in_machine;


};
