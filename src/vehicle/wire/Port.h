#pragma once 
#include <stdint.h>
#include <string>
#include "../../util/Logger.h"
#include <sol.hpp>

class Machine;
class Port;


struct PortValue
{
	enum Type
	{
		NUMBER
	};

	union
	{
		double as_number;
	};

	Type type;

	static std::string get_name(Type type);
	static Type get_type(const std::string& str);

	void call_lua(sol::safe_function& func, Port* port);

	PortValue(double v);
};

struct PortResult
{
	enum ResultType
	{
		GOOD,
		INVALID_TYPE,
		PORT_BLOCKED,
		PORT_NOT_FOUND
	};

	ResultType result;
};

class Port
{

public:

	bool blocked;




	// This is lua's way to differentiate ports,
	// so they need to be unique. This is enforced
	// on port creation
	// (Port names must be unique inside of a part outputs
	// or inputs, you can have two ports named "x"" on the same
	// vehicle, and you can even have two ports named "x" on the same 
	// part, but they cannot be both input or output
	std::string name;

	bool is_output;

	// Only on input ports
	sol::safe_function callback;

	// Only on output ports
	std::vector<Port*> to;

	PortValue::Type type;
	Machine* in_machine;

	void receive(PortValue& val);
};
