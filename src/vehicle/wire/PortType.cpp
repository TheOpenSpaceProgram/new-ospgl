#include "Port.h"

std::string Port::get_name(Port::Type type)
{
	if(type == NUMBER)
	{
		return "number";
	}
	else
	{
		logger->fatal("Should never happen");
		return "unknown";
	}
}

Port::Type Port::get_type(const std::string& str)
{
	if(str == "number")
	{
		return NUMBER;
	}
	else
	{
		logger->fatal("Invalid strin ggiven to PortType::get_type '{}'", str);
		return NUMBER; //< To shut up all compiler
	}
}
