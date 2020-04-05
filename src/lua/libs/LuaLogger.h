#pragma once
#include "../LuaLib.h"
#include <util/Logger.h>

/*
	Alternatives to lua's "print", these functions work with the Logger
	subsystem and write both to the console and to the debug file

	debug(string):
		
		Logs a message only in builds with DEBUG logging enabled

	info(string):

		Logs an information message

	warn(string):

		Logs a warning message

	error(string):

		Logs a error message, but does not cause a lua error

	fatal(string):

		Logs a fatal message and makes the application quit. 
		Only use for fatal errors! Otherwise you can use a lua "error"

*/
class LuaLogger : public LuaLib
{
public:

	virtual void load_to(sol::table& table) override;
};