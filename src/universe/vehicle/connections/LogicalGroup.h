#pragma once
#include <map>
#include <sol/sol.hpp>
#include <string>
#include <functional>

class Machine;

class LogicalGroup
{
public:
	std::string display_str;

	// Bidirectional wires, so if A is connected to B then B is connected to A
	std::unordered_multimap<Machine*, Machine*> connections;

	std::vector<Machine*> get_connected_if(Machine* to, std::function<bool(Machine*)> fnc, bool include_to) const;
	std::vector<Machine*> get_all_connected(Machine* to, bool include_to = true) const;
	std::vector<Machine*> get_connected_with(Machine* to, const std::vector<std::string>& interfaces, bool include_to = true) const;
	std::vector<sol::table> get_connected_interfaces(Machine* to, const std::string& type, bool include_to = true) const;

};
