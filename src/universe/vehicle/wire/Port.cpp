#include "Port.h"

#include "Port.h"

std::string PortValue::get_name(PortValue::Type type)
{
	if (type == NUMBER)
	{
		return "number";
	}
	else
	{
		logger->fatal("Should never happen");
		return "unknown";
	}
}

PortValue::Type PortValue::get_type(const std::string& str)
{
	if (str == "number")
	{
		return NUMBER;
	}
	else
	{
		logger->fatal("Invalid string given to PortType::get_type '{}'", str);
		return NUMBER; //< To shut up all compiler
	}
}

void PortValue::call_lua(sol::safe_function& func, Port* port)
{
	if (type == NUMBER)
	{
		func(port->name, as_number);
	}
}

PortValue::PortValue(double v)
{
	type = NUMBER;
	as_number = v;
}


void Port::receive(PortValue& val)
{
	blocked = true;

	// This should really never happen
	logger->check(!is_output, "Received on an output port");

	val.call_lua(callback, this);
}
