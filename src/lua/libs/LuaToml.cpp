#include "LuaToml.h"
#include <cpptoml.h>
#include <glm/glm.hpp>
#include "../../util/SerializeUtil.h"
#include "../../util/serializers/glm.h"

// We include some read_to_x functions
#include "util/SerializeUtil.h"
#include "universe/vehicle/Vehicle.h"

void LuaToml::load_to(sol::table& table)
{
	using table_ptr = std::shared_ptr<cpptoml::table>;

	// TODO: Table arrays, arrays, etc...
	table.new_usertype<cpptoml::table>("table",
		"new", []()
		{
			return cpptoml::make_table();
		},
		// insert is a bit of a misnomer, so they are named "set" in lua for clarity
		"set_table", [](table_ptr self, const std::string& key, table_ptr tab)
		{
			self->insert(key, tab);
		},
		"set_number", [](table_ptr self, const std::string& key, double num)
		{
			self->insert(key, num);
		},
		"set_string", [](table_ptr self, const std::string& key, const std::string& str)
		{
			self->insert(key, str);
		},
		"set_bool", [](table_ptr self, const std::string& key, bool val)
		{
			self->insert(key, val);
		},
		"set_vec3", [](table_ptr self, const std::string key, glm::dvec3 val)
		{
			table_ptr n_table = cpptoml::make_table();
			serialize(val, *n_table);
			self->insert(key, n_table);
		},
		"get_table", [](table_ptr self, const std::string& key)
		{
			return self->get_table_qualified(key);
		},
		"get_number", [](table_ptr self, const std::string& key)
		{
			auto t = self->get_qualified_as<double>(key);
			if(t)
				return sol::optional(*t);
			else
				return sol::optional<double>();
		},
		"get_number_or", [](table_ptr self, const std::string& key, double def_val)
		{
			return self->get_qualified_as<double>(key).value_or(def_val);
		},
		"get_string", [](table_ptr self, const std::string& key)
		{
			auto t = self->get_qualified_as<std::string>(key);
			if(t)
				return sol::optional(*t);
			else
				return sol::optional<std::string>();
		},
		"get_string_or", [](table_ptr self, const std::string& key, const std::string& def_val)
		{
			return self->get_qualified_as<std::string>(key).value_or(def_val);
		},
		"get_bool", [](table_ptr self, const std::string& key)
		{
			auto t = self->get_qualified_as<bool>(key);
			if(t)
				return sol::optional(*t);
			else
				return sol::optional<bool>();
		},
		"get_bool_or", [](table_ptr self, const std::string& key, bool def_val)
		{
			return self->get_qualified_as<bool>(key).value_or(def_val);
		},
		// TODO: Implement nil return for these
		"get_vec3", [](table_ptr self, const std::string& key)
		{
			glm::dvec3 v; deserialize(v, *self->get_table(key));
			return v;
		},
		"get_vec3_or", [](table_ptr self, const std::string& key, glm::dvec3 def_val)
		{
			table_ptr tbl = self->get_table(key);
			if (tbl)
			{
				glm::dvec3 v; deserialize(v, *tbl);
				return v;
			}
			else
			{
				return def_val;
			}
		},
		"get_array_of_string", [](table_ptr self, const std::string& key)
		{
			return sol::as_table(*self->get_array_of<std::string>(key));	
		},
		"get_array_of_number", [](table_ptr self, const std::string& key)
		{
			return sol::as_table(*self->get_array_of<double>(key));
		},
		"get_array_of_table", [](table_ptr self, const std::string& key)
	   	{
			// Here we simply return an empty array, not nil
			auto val = self->get_table_array_qualified(key);
			std::vector<std::shared_ptr<cpptoml::table>> tables;
			if(val)
			{
				tables = val->get();
			}

			return sol::as_table(tables);
	   	},
	   	"read_to_vehicle", [](table_ptr self, Vehicle& target)
	   	{
			SerializeUtil::read_to(*self, target);
	   	},
		"erase", [](table_ptr self, const std::string& key)
		{
			self->erase(key);
		}
	);


}
