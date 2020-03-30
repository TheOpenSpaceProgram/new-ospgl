#include "LuaToml.h"
#include <cpptoml.h>

void LuaToml::load_to(sol::table& table)
{
	using table_ptr = std::shared_ptr<cpptoml::table>;

	// TODO: Table arrays, arrays, etc...
	table.new_usertype<cpptoml::table>("table",
		"new", []()
		{
			return cpptoml::make_table();
		},
		"insert_table", [](table_ptr self, const std::string& key, table_ptr tab)
		{
			self->insert(key, tab);
		},
		"insert_number", [](table_ptr self, const std::string& key, double num)
		{
			self->insert(key, num);
		},
		"insert_string", [](table_ptr self, const std::string& key, const std::string& str)
		{
			self->insert(key, str);
		},
		"insert_bool", [](table_ptr self, const std::string& key, bool val)
		{
			self->insert(key, val);
		},
		"get_table", [](table_ptr self, const std::string& key)
		{
			return self->get_table_qualified(key);
		},
		"get_number", [](table_ptr self, const std::string& key)
		{
			return *self->get_qualified_as<double>(key);
		},
		"get_number_or", [](table_ptr self, const std::string& key, double def_val)
		{
			return self->get_qualified_as<double>(key).value_or(def_val);
		},
		"get_string", [](table_ptr self, const std::string& key)
		{
			return *self->get_qualified_as<std::string>(key);
		},
		"get_string_or", [](table_ptr self, const std::string& key, const std::string& def_val)
		{
			return self->get_qualified_as<std::string>(key).value_or(def_val);
		},
		"get_bool", [](table_ptr self, const std::string& key)
		{
			return *self->get_qualified_as<bool>(key);
		},
		"get_bool_or", [](table_ptr self, const std::string& key, bool def_val)
		{
			return self->get_qualified_as<bool>(key).value_or(def_val);
		},
		"erase", [](table_ptr self, const std::string& key)
		{
			self->erase(key);	
		}	
	);


}
