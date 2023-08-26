#include "LogicalGroup.h"
#include "../part/Machine.h"


std::vector<Machine*> LogicalGroup::get_all_connected(Machine* self, bool include_this) const
{
	// TODO: We could cache this? Could be a small perfomance gain
	return get_connected_if(self, [](Machine* m){ return true; }, include_this);
}

std::vector<Machine*> LogicalGroup::get_connected_with(Machine* self, const std::vector<std::string>& interfaces, bool include_this) const
{
	return get_connected_if(self, [interfaces](Machine* m)
							{
								for(const std::string& a : interfaces)
								{
									if(m->interfaces.find(a) != m->interfaces.end())
									{
										return true;
									}
								}

								return false;
							}, include_this);
}

std::vector<sol::table> LogicalGroup::get_connected_interfaces(Machine* self, const std::string& type, bool include_this) const
{
	std::vector<Machine*> machines = get_connected_with(self, {type}, include_this);
	std::vector<sol::table> out;
	out.reserve(machines.size());

	for(Machine* m : machines)
	{
		out.push_back(m->interfaces[type]);
	}

	return out;
}

std::vector<Machine*> LogicalGroup::get_connected_if(Machine* self, std::function<bool(Machine*)> fnc, bool include_this) const
{
	std::vector<Machine*> out;

	auto range = connections.equal_range(self);
	for(auto it = range.first; it != range.second; it++)
	{
		if(fnc(it->second))
		{
			out.push_back(it->second);
		}
	}

	if(include_this && fnc(self))
	{
		out.push_back(self);
	}

	return out;
}
